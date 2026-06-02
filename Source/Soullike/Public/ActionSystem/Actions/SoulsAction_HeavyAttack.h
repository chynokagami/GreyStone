#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/SoulsAction.h"
#include "SoulsAction_HeavyAttack.generated.h"


class UAnimMontage;

UCLASS()
class SOULLIKE_API USoulsAction_HeavyAttack : public USoulsAction
{
    GENERATED_BODY()

public:

    USoulsAction_HeavyAttack();

    virtual void StartAction_Implementation() override;
    virtual void StopAction_Implementation() override;

protected:

    /** Fallback montage if the player can't supply a charge-level-appropriate one (debug, AI). */
    UPROPERTY(EditDefaultsOnly, Category="HeavyAttack")
    TObjectPtr<UAnimMontage> FallbackMontage;

    void OnAttackTimerElapsed();

    FTimerHandle AttackTimerHandle;
};