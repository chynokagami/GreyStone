#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BehaviorTreeTypes.h"   // FBlackboardKeySelector
#include "GameplayTagContainer.h"             // FGameplayTagContainer
#include "SoulsBTService_FaceTarget.generated.h"

UCLASS()
class SOULLIKE_API USoulsBTService_FaceTarget : public UBTService
{
    GENERATED_BODY()

public:
    USoulsBTService_FaceTarget();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp,
                          uint8* NodeMemory,
                          float DeltaSeconds) override;

    // BB key selector
    UPROPERTY(EditAnywhere, Category = "Souls|AI")
    FBlackboardKeySelector TargetActorKey;

    // Tunables
    UPROPERTY(EditAnywhere, Category = "Souls|AI",
              meta = (ClampMin = "30.0", ClampMax = "900.0"))
    float YawSpeedDegPerSec = 360.0f;

    UPROPERTY(EditAnywhere, Category = "Souls|AI",
              meta = (ClampMin = "0.5", ClampMax = "30.0"))
    float DeadzoneDegrees = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Souls|AI")
    FGameplayTagContainer BlockingTags;
};