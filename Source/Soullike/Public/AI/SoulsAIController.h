#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SoulsAIController.generated.h"

class UBehaviorTree;
class UBlackboardData;
class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
struct FAIStimulus;

UCLASS()
class SOULLIKE_API ASoulsAIController : public AAIController
{
    GENERATED_BODY()

public:
    ASoulsAIController();

protected:
    //~ AAIController
    virtual void OnPossess(APawn* InPawn) override;

    virtual void OnUnPossess() override;
    //~ End AAIController

    // BT / Blackboard config

    /** BehaviorTree this controller runs after Possess. Set in BP CDO per archetype. */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI")
    TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

    /** Blackboard data asset. Must contain at minimum the keys this controller writes (TargetActor). */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI")
    TObjectPtr<UBlackboardData> DefaultBlackboard;

    UPROPERTY(VisibleAnywhere, Category = "Souls|AI|Perception")
    TObjectPtr<UAIPerceptionComponent> AIPerceptionComp;

    UPROPERTY(VisibleAnywhere, Category = "Souls|AI|Perception")
    TObjectPtr<UAISenseConfig_Sight> SightConfig;

    UPROPERTY(VisibleAnywhere, Category = "Souls|AI|Perception")
    TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

    UFUNCTION()
    void OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Tunables (mirror to UAISenseConfig in constructor)
    // Exposed here, not on the SenseConfig directly, so per-archetype BP CDO
    // can override them without dragging users into nested object editors.
    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Sight", meta = (ClampMin = "100.0"))
    float SightRadius = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Sight", meta = (ClampMin = "100.0"))
    float SightLoseRadius = 2000.0f;

    /** Half-angle of vision cone in degrees. 60 = 120 total, classic enemy FOV. */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float SightPeripheralAngleDeg = 60.0f;

    /** How long a sighted target is remembered after losing line of sight (seconds). */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Sight", meta = (ClampMin = "0.0"))
    float SightMaxAge = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Hearing", meta = (ClampMin = "100.0"))
    float HearingRange = 1200.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Hearing",
              meta = (ClampMin = "0.1", UIMin = "0.1"))
    float NoiseInvestigationMemorySeconds = 8.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI|Perception|Sight",
              meta = (ClampMin = "0.0", UIMin = "0.0"))
    float LostSightGracePeriodSeconds = 3.0f;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> PendingLostTarget;

    FTimerHandle LostSightClearTimer;

    /** Clears LastNoiseLocation after the investigation memory window expires. */
    FTimerHandle NoiseInvestigationClearTimer;

    UFUNCTION()
    void HandleLostSightTimerElapsed();

    UFUNCTION()
    void HandleNoiseInvestigationTimerElapsed();
};
