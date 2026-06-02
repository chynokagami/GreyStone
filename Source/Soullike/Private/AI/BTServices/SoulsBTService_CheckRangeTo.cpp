#include "AI/BTServices/SoulsBTService_CheckRangeTo.h"

#include "Soullike.h"                            // LogGame
#include "AI/BlackboardKeys.h"                   // SoulsBBKeys::*
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "SharedGameplayTags.h"

USoulsBTService_CheckRangeTo::USoulsBTService_CheckRangeTo()
{
    // Display name in the BT editor's node header.
    NodeName = TEXT("Check Range / LOS To Target");

    // Default tick cadence. BTService runs at this interval (with optional
    // RandomDeviation jitter) while its parent composite is active. 0.2s is
    // fast enough for melee combat decisions without flooding the BT.
    Interval = 0.2f;
    RandomDeviation = 0.05f;

    // Default BB key selections
    TargetActorKey.SelectedKeyName       = SoulsBBKeys::TargetActor;
    bIsInAttackRangeKey.SelectedKeyName  = SoulsBBKeys::bIsInAttackRange;
    bHasLineOfSightKey.SelectedKeyName   = SoulsBBKeys::bHasLineOfSight;
    DistanceToTargetKey.SelectedKeyName  = SoulsBBKeys::DistanceToTarget;
    TargetAngleAbsKey.SelectedKeyName    = SoulsBBKeys::TargetAngleAbs;
    HealthRatioKey.SelectedKeyName       = SoulsBBKeys::HealthRatio;
    bShouldAttackKey.SelectedKeyName     = SoulsBBKeys::bShouldAttack;
    bShouldStrafeKey.SelectedKeyName     = SoulsBBKeys::bShouldStrafe;
    bShouldFlankKey.SelectedKeyName      = SoulsBBKeys::bShouldFlank;
    bShouldRetreatKey.SelectedKeyName    = SoulsBBKeys::bShouldRetreat;

    TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, TargetActorKey), AActor::StaticClass());
    bIsInAttackRangeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bIsInAttackRangeKey));
    bHasLineOfSightKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bHasLineOfSightKey));
    DistanceToTargetKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, DistanceToTargetKey));
    TargetAngleAbsKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, TargetAngleAbsKey));
    HealthRatioKey.AddFloatFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, HealthRatioKey));
    bShouldAttackKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bShouldAttackKey));
    bShouldStrafeKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bShouldStrafeKey));
    bShouldFlankKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bShouldFlankKey));
    bShouldRetreatKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(USoulsBTService_CheckRangeTo, bShouldRetreatKey));
}

void USoulsBTService_CheckRangeTo::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
    if (!ensure(BBComp))
    {
        return;
    }

    AAIController* Controller = OwnerComp.GetAIOwner();
    if (!ensure(Controller))
    {
        return;
    }

    // OwningPawn can be null on the spawn frame before OnPossess completes,
    // or briefly after death-but-before-corpse-GC. Both are non-fatal.
    APawn* OwningPawn = Controller->GetPawn();
    if (!OwningPawn)
    {
        return;
    }

    // Read target
    AActor* TargetActor = Cast<AActor>(BBComp->GetValueAsObject(TargetActorKey.SelectedKeyName));

    if (!TargetActor)
    {
        // No target -> both flags false. Without this branch, the BT could
        // keep stale "in range" reading after AIPerception cleared TargetActor.
        BBComp->SetValueAsBool(bIsInAttackRangeKey.SelectedKeyName, false);
        BBComp->SetValueAsBool(bHasLineOfSightKey.SelectedKeyName, false);
        BBComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, 0.0f);
        BBComp->SetValueAsFloat(TargetAngleAbsKey.SelectedKeyName, 0.0f);
        BBComp->SetValueAsFloat(HealthRatioKey.SelectedKeyName, 1.0f);
        BBComp->SetValueAsBool(bShouldAttackKey.SelectedKeyName, false);
        BBComp->SetValueAsBool(bShouldStrafeKey.SelectedKeyName, false);
        BBComp->SetValueAsBool(bShouldFlankKey.SelectedKeyName, false);
        BBComp->SetValueAsBool(bShouldRetreatKey.SelectedKeyName, false);
        return;
    }

    // Distance check
    const FVector PawnLocation = OwningPawn->GetActorLocation();
    const FVector TargetLocation = TargetActor->GetActorLocation();
    const float DistanceToTarget = FVector::Dist(TargetLocation, PawnLocation);
    const float DistSq = FMath::Square(DistanceToTarget);

    const bool bWasInRange = BBComp->GetValueAsBool(bIsInAttackRangeKey.SelectedKeyName);
    const float AttackRangeExit = AttackRange + FMath::Max(0.0f, AttackRangeHysteresis);
    const float EffectiveAttackRange = bWasInRange ? AttackRangeExit : AttackRange;
    const bool bInRange = DistSq < FMath::Square(EffectiveAttackRange);

    // Facing angle
    const FVector ToTarget = (TargetLocation - PawnLocation).GetSafeNormal2D();
    const FVector PawnForward = OwningPawn->GetActorForwardVector().GetSafeNormal2D();
    const float FacingDot = FVector::DotProduct(PawnForward, ToTarget);
    const float TargetAngleAbs = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FacingDot, -1.0f, 1.0f)));

    // Line of sight check
    const bool bHasLOS = Controller->LineOfSightTo(TargetActor);

    // Health context
    float HealthRatio = 1.0f;
    if (const USoulsActionSystemComponent* ASC = OwningPawn->FindComponentByClass<USoulsActionSystemComponent>())
    {
        const float Health = ASC->GetAttributeValue(SharedGameplayTags::Attribute_Health);
        const float HealthMax = ASC->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);
        if (HealthMax > KINDA_SMALL_NUMBER)
        {
            HealthRatio = FMath::Clamp(Health / HealthMax, 0.0f, 1.0f);
        }
    }

    // Intent hints
    const bool bFacingForAttack = TargetAngleAbs <= AttackFacingAngleDeg;
    const bool bLowHealthThreatened = HealthRatio <= RetreatHealthRatio && DistanceToTarget <= RetreatThreatRange;
    const bool bShouldRetreat = bLowHealthThreatened && bHasLOS;
    const bool bShouldAttack = !bShouldRetreat && bInRange && bHasLOS && bFacingForAttack;
    const bool bMidRange = DistanceToTarget > AttackRange && DistanceToTarget <= StrafeRange;
    const bool bLongRange = DistanceToTarget > StrafeRange && DistanceToTarget <= FlankRange;
    const float DecisionHold = FMath::Max(0.1f, DecisionHoldSeconds);
    const UWorld* World = OwningPawn->GetWorld();
    const float WorldTime = World ? World->GetTimeSeconds() : 0.0f;
    const int32 DecisionBucket = FMath::FloorToInt(WorldTime / DecisionHold);
    const uint32 PawnSeed = GetTypeHash(OwningPawn->GetFName());
    FRandomStream DecisionRng(static_cast<int32>(PawnSeed ^ static_cast<uint32>(DecisionBucket)));
    const bool bPickedStrafe = DecisionRng.FRand() <= StrafeChance;
    const bool bPickedFlank = DecisionRng.FRand() <= FlankChance;
    const bool bPickedCloseReposition = DecisionRng.FRand() <= CloseAngleRepositionChance;
    const bool bShouldStrafe = !bShouldAttack && !bShouldRetreat && bHasLOS && bMidRange && bPickedStrafe;
    const bool bShouldFlank = !bShouldAttack && !bShouldRetreat && bHasLOS
        && (
            bShouldStrafe
            || (bLongRange && bPickedFlank)
            || (bInRange && !bFacingForAttack && bPickedCloseReposition)
            || (!bInRange && !bFacingForAttack && bPickedFlank)
        );

    // Write back
    BBComp->SetValueAsBool(bIsInAttackRangeKey.SelectedKeyName, bInRange);
    BBComp->SetValueAsBool(bHasLineOfSightKey.SelectedKeyName, bHasLOS);
    BBComp->SetValueAsFloat(DistanceToTargetKey.SelectedKeyName, DistanceToTarget);
    BBComp->SetValueAsFloat(TargetAngleAbsKey.SelectedKeyName, TargetAngleAbs);
    BBComp->SetValueAsFloat(HealthRatioKey.SelectedKeyName, HealthRatio);
    BBComp->SetValueAsBool(bShouldAttackKey.SelectedKeyName, bShouldAttack);
    BBComp->SetValueAsBool(bShouldStrafeKey.SelectedKeyName, bShouldStrafe);
    BBComp->SetValueAsBool(bShouldFlankKey.SelectedKeyName, bShouldFlank);
    BBComp->SetValueAsBool(bShouldRetreatKey.SelectedKeyName, bShouldRetreat);
}
