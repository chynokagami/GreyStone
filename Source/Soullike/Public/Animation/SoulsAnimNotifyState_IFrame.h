#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulsAnimNotifyState_IFrame.generated.h"

UCLASS(meta=(DisplayName="Souls IFrame Window"))
class SOULLIKE_API USoulsAnimNotifyState_IFrame : public UAnimNotifyState
{
    GENERATED_BODY()

public:

    USoulsAnimNotifyState_IFrame();

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
                             UAnimSequenceBase* Animation,
                             float TotalDuration,
                             const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
                           UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference) override;
};