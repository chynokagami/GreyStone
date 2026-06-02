#include "AI/BTTasks/SoulsBTTask_Attack.h"

#include "Soullike.h"                                  // LogGame
#include "SharedGameplayTags.h"                        // SharedGameplayTags::Action_*, Status_*
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "AIController.h"
#include "AI/BlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/Pawn.h"

USoulsBTTask_Attack::USoulsBTTask_Attack()
{
    NodeName = TEXT("Start Souls Action");

    // Latent task setup
    bNotifyTick = true;

    // Default action: light melee attack. Designers retarget per-pawn in
    // BT subclass / per-node placement (e.g. heavy slam for boss BT).
    ActionToStart = SharedGameplayTags::Action_LightAttack;

    // Default monitor: Status.IsAttacking. Both LightAttack and HeavyAttack
    // BP CDOs grant this tag, so this default works for the common case.
    MonitorTag = SharedGameplayTags::Status_IsAttacking;

    RequiredBoolKey.SelectedKeyName = SoulsBBKeys::bShouldAttack;
    RequiredBoolKey.AddBoolFilter(
        this,
        GET_MEMBER_NAME_CHECKED(USoulsBTTask_Attack, RequiredBoolKey));
}

EBTNodeResult::Type USoulsBTTask_Attack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Resolve owning pawn + ASC
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!ensure(AICon))
    {
        return EBTNodeResult::Failed;
    }

    APawn* OwningPawn = AICon->GetPawn();
    if (!OwningPawn)
    {
        // possess race / post-death frame; non-fatal
        return EBTNodeResult::Failed;
    }

    USoulsActionSystemComponent* ASC =
        OwningPawn->FindComponentByClass<USoulsActionSystemComponent>();
    if (!ensure(ASC))
    {
        // Misconfigured pawn: AI without ASC. ensure() warns once.
        return EBTNodeResult::Failed;
    }

    // Validate config
    if (!ActionToStart.IsValid() || !MonitorTag.IsValid())
    {
        UE_LOG(LogGame, Warning,
            TEXT("[BTTask_Attack] %s: ActionToStart or MonitorTag not configured"),
            *OwningPawn->GetName());
        return EBTNodeResult::Failed;
    }

    if (bUseRequiredBoolKey)
    {
        const UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
        if (!BBComp || !BBComp->GetValueAsBool(RequiredBoolKey.SelectedKeyName))
        {
            return EBTNodeResult::Failed;
        }
    }

    // Initialize per-instance memory
    FSoulsBTTask_AttackMemory* Mem = GetMemory(NodeMemory);
    Mem->TaskStartTime    = OwningPawn->GetWorld()->TimeSeconds;
    Mem->bActionStarted   = false;

    // Try to start the action
    ASC->StartAction(ActionToStart);

    if (!ASC->ActiveGameplayTags.HasTag(MonitorTag))
    {
        // Action did not start — log so designers see why during BT debugging,
        // and fail this task immediately so the BT picks the next branch.
        UE_LOG(LogGame, Verbose,
            TEXT("[BTTask_Attack] %s: StartAction(%s) rejected (cooldown/cost/blocked?)"),
            *OwningPawn->GetName(), *ActionToStart.ToString());
        return EBTNodeResult::Failed;
    }

    Mem->bActionStarted = true;

    UE_LOG(LogGame, Verbose,
        TEXT("[BTTask_Attack] %s: started %s, monitoring %s"),
        *OwningPawn->GetName(), *ActionToStart.ToString(), *MonitorTag.ToString());

    return EBTNodeResult::InProgress;
}

void USoulsBTTask_Attack::TickTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* OwningPawn = AICon ? AICon->GetPawn() : nullptr;
    if (!OwningPawn)
    {
        // pawn died or got unpossessed mid-task — finish so BT moves on
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    USoulsActionSystemComponent* ASC =
        OwningPawn->FindComponentByClass<USoulsActionSystemComponent>();
    if (!ASC)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    const FSoulsBTTask_AttackMemory* Mem = GetMemory(NodeMemory);

    // Completion check
    if (!ASC->ActiveGameplayTags.HasTag(MonitorTag))
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // Timeout safety net
    const float Elapsed = OwningPawn->GetWorld()->TimeSeconds - Mem->TaskStartTime;
    if (Elapsed > TimeoutSeconds)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[BTTask_Attack] %s: action %s exceeded %.1fs timeout, force-stopping"),
            *OwningPawn->GetName(), *ActionToStart.ToString(), TimeoutSeconds);

        ASC->StopAction(ActionToStart);
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
}

EBTNodeResult::Type USoulsBTTask_Attack::AbortTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    Super::AbortTask(OwnerComp, NodeMemory);

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (APawn* OwningPawn = (AICon ? AICon->GetPawn() : nullptr))
    {
        if (USoulsActionSystemComponent* ASC =
                OwningPawn->FindComponentByClass<USoulsActionSystemComponent>())
        {
            const FSoulsBTTask_AttackMemory* Mem = GetMemory(NodeMemory);
            if (Mem->bActionStarted)
            {
                ASC->StopAction(ActionToStart);
            }
        }
    }

    return EBTNodeResult::Aborted;
}

uint16 USoulsBTTask_Attack::GetInstanceMemorySize() const
{
    return sizeof(FSoulsBTTask_AttackMemory);
}

void USoulsBTTask_Attack::InitializeMemory(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
    new (NodeMemory) FSoulsBTTask_AttackMemory();
}

void USoulsBTTask_Attack::CleanupMemory(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
    GetMemory(NodeMemory)->~FSoulsBTTask_AttackMemory();
}

FString USoulsBTTask_Attack::GetStaticDescription() const
{
    FString Description = FString::Printf(
        TEXT("Start [%s] until [%s] clears (timeout %.1fs)"),
        ActionToStart.IsValid()  ? *ActionToStart.ToString()  : TEXT("?"),
        MonitorTag.IsValid()     ? *MonitorTag.ToString()     : TEXT("?"),
        TimeoutSeconds);

    if (bUseRequiredBoolKey)
    {
        Description += FString::Printf(
            TEXT("\nRequires BB [%s] = true"),
            *RequiredBoolKey.SelectedKeyName.ToString());
    }

    return Description;
}
