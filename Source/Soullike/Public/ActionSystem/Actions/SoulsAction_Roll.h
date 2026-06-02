#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/SoulsAction.h"
#include "SoulsAction_Roll.generated.h"


class UAnimMontage;

UCLASS()
class SOULLIKE_API USoulsAction_Roll : public USoulsAction
{
    GENERATED_BODY()

public:

    USoulsAction_Roll();

    virtual void StartAction_Implementation() override;
    virtual void StopAction_Implementation() override;

protected:

    /** Required: roll motion. Should have Root Motion enabled in the montage settings. */
    UPROPERTY(EditDefaultsOnly, Category="Roll")
    TObjectPtr<UAnimMontage> RollMontage;

    void OnRollTimerElapsed();

    FTimerHandle RollTimerHandle;
};