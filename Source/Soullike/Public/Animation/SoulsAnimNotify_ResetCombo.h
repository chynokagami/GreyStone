#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SoulsAnimNotify_ResetCombo.generated.h"

UCLASS(meta=(DisplayName="Souls Reset Combo"))
class SOULLIKE_API USoulsAnimNotify_ResetCombo : public UAnimNotify
{
    GENERATED_BODY()

public:
    USoulsAnimNotify_ResetCombo();

    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return TEXT("Souls ResetCombo");
    }
};