#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/SoulsAction.h"
#include "SoulsAction_LightAttack.generated.h"


class UAnimMontage;

UCLASS()
class SOULLIKE_API USoulsAction_LightAttack : public USoulsAction
{
    GENERATED_BODY()

public:

    USoulsAction_LightAttack();

    virtual void StartAction_Implementation() override;
    virtual void StopAction_Implementation() override;

protected:

    UPROPERTY(EditDefaultsOnly, Category="LightAttack")
    TObjectPtr<UAnimMontage> AttackMontage;

    void OnAttackTimerElapsed();

    FTimerHandle AttackTimerHandle;
};