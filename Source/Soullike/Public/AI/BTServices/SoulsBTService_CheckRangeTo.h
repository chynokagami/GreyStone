#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"   // FBlackboardKeySelector
#include "SoulsBTService_CheckRangeTo.generated.h"

UCLASS()
class SOULLIKE_API USoulsBTService_CheckRangeTo : public UBTService
{
    GENERATED_BODY()

public:
    USoulsBTService_CheckRangeTo();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // BB key selectors
    /** Object/Actor BB key. Read by this service. Defaults to SoulsBBKeys::TargetActor. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI")
    FBlackboardKeySelector TargetActorKey;

    /** Bool BB key. Written each tick. Defaults to SoulsBBKeys::bIsInAttackRange. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI")
    FBlackboardKeySelector bIsInAttackRangeKey;

    /** Bool BB key. Written each tick. Defaults to SoulsBBKeys::bHasLineOfSight. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI")
    FBlackboardKeySelector bHasLineOfSightKey;

    /** Float BB key. Distance to current target in cm. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Context")
    FBlackboardKeySelector DistanceToTargetKey;

    /** Float BB key. Absolute facing angle to target in degrees. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Context")
    FBlackboardKeySelector TargetAngleAbsKey;

    /** Float BB key. Health divided by max health, clamped to [0, 1]. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Context")
    FBlackboardKeySelector HealthRatioKey;

    /** Bool BB key. Attack branch hint. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent")
    FBlackboardKeySelector bShouldAttackKey;

    /** Bool BB key. Strafe/orbit branch hint. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent")
    FBlackboardKeySelector bShouldStrafeKey;

    /** Bool BB key. Flank/reposition branch hint. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent")
    FBlackboardKeySelector bShouldFlankKey;

    /** Bool BB key. Retreat/disengage branch hint. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent")
    FBlackboardKeySelector bShouldRetreatKey;

    // Tunables
    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float AttackRange = 250.0f;

    /** Extra exit buffer for bIsInAttackRange to avoid flickering on the edge. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float AttackRangeHysteresis = 60.0f;

    /** Distance where the AI should stop running straight at the target and start orbiting. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float StrafeRange = 450.0f;

    /** Distance beyond this pushes the AI toward flank/reposition movement. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float FlankRange = 700.0f;

    /** Health ratio below which the AI prefers a defensive retreat when pressured. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RetreatHealthRatio = 0.35f;

    /** Retreated only when the target is close enough to be a real threat. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", UIMin = "0.0"))
    float RetreatThreatRange = 360.0f;

    /** Attack only when the AI is roughly facing the target. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float AttackFacingAngleDeg = 55.0f;

    /** Random chance to choose strafe instead of direct chase while in mid range. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StrafeChance = 0.35f;

    /** Random chance to choose a flank reposition while outside attack range. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FlankChance = 0.45f;

    /** Chance to reposition instead of waiting when the target is close but outside the attack facing cone. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CloseAngleRepositionChance = 0.45f;

    /** Holds randomized intent choices for this long to avoid BT branch jitter. */
    UPROPERTY(EditAnywhere, Category = "Souls|AI|Intent", meta = (ClampMin = "0.1", UIMin = "0.1"))
    float DecisionHoldSeconds = 1.5f;
};
