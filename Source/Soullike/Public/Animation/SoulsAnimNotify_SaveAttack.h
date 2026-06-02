#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "SoulsAnimNotify_SaveAttack.generated.h"

UCLASS(meta=(DisplayName="Souls Save Attack"))
class SOULLIKE_API USoulsAnimNotify_SaveAttack : public UAnimNotify
{
    GENERATED_BODY()

public:
    USoulsAnimNotify_SaveAttack();

    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return TEXT("Souls SaveAttack");
    }
};