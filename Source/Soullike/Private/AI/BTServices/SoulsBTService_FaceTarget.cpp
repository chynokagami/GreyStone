#include "AI/BTServices/SoulsBTService_FaceTarget.h"

#include "Soullike.h"                            // LogGame
#include "AI/BlackboardKeys.h"                   // SoulsBBKeys::TargetActor
#include "SharedGameplayTags.h"                  // Status_Is* tags
#include "ActionSystem/SoulsActionSystemComponent.h"  // ASC.ActiveGameplayTags
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"


USoulsBTService_FaceTarget::USoulsBTService_FaceTarget()
{
    NodeName = TEXT("Face Target");

    Interval = 0.2f;
    RandomDeviation = 0.05f;

    // Default BB key selection
    TargetActorKey.SelectedKeyName = SoulsBBKeys::TargetActor;

    TargetActorKey.AddObjectFilter(
        this,
        GET_MEMBER_NAME_CHECKED(USoulsBTService_FaceTarget, TargetActorKey),
        AActor::StaticClass());

    BlockingTags.AddTag(SharedGameplayTags::Status_IsAttacking);
    BlockingTags.AddTag(SharedGameplayTags::Status_IsRolling);
    BlockingTags.AddTag(SharedGameplayTags::Status_IsBlocking);
    BlockingTags.AddTag(SharedGameplayTags::Status_IsDead);
}


void USoulsBTService_FaceTarget::TickNode(UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    // Resolve owning pawn
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        // Service ran on a non-AI BT? Misconfigured. Silent return — ensure()
        // would spam every 0.2s.
        return;
    }

    APawn* OwningPawn = AICon->GetPawn();
    if (!OwningPawn)
    {
        // Possess race on spawn frame, or post-death-pre-corpse-GC. Non-fatal.
        return;
    }

    // Resolve BB target
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TargetActorKey.SelectedKeyName));
    if (!Target)
    {
        // No target = nothing to face. This is the steady state when AI is
        // patrolling or has lost the player and Sight Max Age expired.
        // Silent return — patrol logic, if added later, owns rotation.
        return;
    }

    // Check BlockingTags
    USoulsActionSystemComponent* ASC =
        OwningPawn->FindComponentByClass<USoulsActionSystemComponent>();
    if (ASC && ASC->ActiveGameplayTags.HasAny(BlockingTags))
    {
        return;
    }

    // Compute target yaw
    const FVector ToTarget =
        (Target->GetActorLocation() - OwningPawn->GetActorLocation()).GetSafeNormal2D();
    if (ToTarget.IsNearlyZero())
    {
        // Pawns overlapping (target standing on AI). Degenerate; skip
        // without rotating to avoid undefined yaw direction.
        return;
    }

    const FRotator CurrentRot = OwningPawn->GetActorRotation();
    const FRotator TargetRot(0.0f, ToTarget.Rotation().Yaw, 0.0f);

    // Deadzone check
    const float YawDelta = FMath::Abs(
        FMath::FindDeltaAngleDegrees(CurrentRot.Yaw, TargetRot.Yaw));
    if (YawDelta < DeadzoneDegrees)
    {
        return;
    }

    const FRotator NewRot = FMath::RInterpConstantTo(
        CurrentRot, TargetRot, DeltaSeconds, YawSpeedDegPerSec);

    OwningPawn->SetActorRotation(NewRot);
}