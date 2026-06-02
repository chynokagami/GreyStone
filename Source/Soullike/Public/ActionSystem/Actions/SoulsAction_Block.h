#pragma once

#include "CoreMinimal.h"
#include "ActionSystem/SoulsAction.h"
#include "SoulsAction_Block.generated.h"


class UAnimMontage;

UCLASS()
class SOULLIKE_API USoulsAction_Block : public USoulsAction
{
    GENERATED_BODY()

public:

    USoulsAction_Block();

    virtual void StartAction_Implementation() override;
    virtual void StopAction_Implementation() override;

protected:

    /** Walk-speed multiplier while blocking. 0.5 = 50% normal speed. */
    UPROPERTY(EditDefaultsOnly, Category="Block", meta=(ClampMin="0.0", ClampMax="1.0"))
    float BlockSpeedMultiplier = 0.5f;

    /** Optional one-shot "raise shield / guard pose" montage. Looping not required. */
    UPROPERTY(EditDefaultsOnly, Category="Block")
    TObjectPtr<UAnimMontage> BlockMontage;

    /** Blend-out time when StopAction stops the montage. */
    UPROPERTY(EditDefaultsOnly, Category="Block", meta=(ClampMin="0.0"))
    float MontageBlendOut = 0.15f;

    /** Cached at StartAction, restored at StopAction. Survives external StopAction (guard break). */
    float CachedMaxWalkSpeed = 0.f;
};