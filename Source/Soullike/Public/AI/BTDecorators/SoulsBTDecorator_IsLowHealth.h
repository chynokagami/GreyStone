#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "SoulsBTDecorator_IsLowHealth.generated.h"

UCLASS()
class SOULLIKE_API USoulsBTDecorator_IsLowHealth : public UBTDecorator
{
    GENERATED_BODY()

public:
    USoulsBTDecorator_IsLowHealth();

protected:
    UPROPERTY(EditAnywhere, Category = "Souls|AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LowHealthFraction = 0.3f;

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

    virtual FString GetStaticDescription() const override;
};