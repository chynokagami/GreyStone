#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "GameplayTagContainer.h"
#include "SoulsAnimNotify_DamageWindow.generated.h"

class ASoulsWeaponBase;

UCLASS(meta=(DisplayName="DamageWindow"))
class SOULLIKE_API USoulsAnimNotify_DamageWindow : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    USoulsAnimNotify_DamageWindow();

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              float TotalDuration, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override;

protected:

    UPROPERTY(EditAnywhere, Category="Damage")
    float DamageAmount = 30.0f;

    UPROPERTY(EditAnywhere, Category="Damage", meta=(Categories="Damage"))
    FGameplayTag DamageType;

private:

    /** Walk Mesh -> Owner -> Cast<ASoulsBaseCharacter> -> EquippedWeapon. Returns null if any step fails. */
    ASoulsWeaponBase* ResolveWeapon(USkeletalMeshComponent* MeshComp, bool bLogMissingWeapon = true) const;

};
