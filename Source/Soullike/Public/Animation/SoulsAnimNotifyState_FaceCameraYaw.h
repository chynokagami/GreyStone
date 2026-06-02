#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "SoulsAnimNotifyState_FaceCameraYaw.generated.h"

UCLASS(meta=(DisplayName="Souls Face Camera Yaw"))
class SOULLIKE_API USoulsAnimNotifyState_FaceCameraYaw : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    USoulsAnimNotifyState_FaceCameraYaw();

    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp,
                             UAnimSequenceBase* Animation,
                             float TotalDuration,
                             const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyTick(USkeletalMeshComponent* MeshComp,
                            UAnimSequenceBase* Animation,
                            float FrameDeltaTime,
                            const FAnimNotifyEventReference& EventReference) override;

    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp,
                           UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference) override;

protected:
    /** Higher values rotate faster toward camera yaw. */
    UPROPERTY(EditAnywhere, Category="Face Camera", meta=(ClampMin="0.0"))
    float InterpSpeed = 12.f;

    /** Snap when yaw delta is already within this tolerance. */
    UPROPERTY(EditAnywhere, Category="Face Camera", meta=(ClampMin="0.0", ClampMax="180.0"))
    float SnapToleranceDegrees = 1.f;

    /** Temporarily disable movement-facing while the notify is active, then restore it on end. */
    UPROPERTY(EditAnywhere, Category="Face Camera")
    bool bDisableOrientRotationToMovement = true;

    /** Prefer controller/camera yaw. If false or unavailable, falls back to actor rotation. */
    UPROPERTY(EditAnywhere, Category="Face Camera")
    bool bUseControllerYaw = true;

private:
    ACharacter* ResolveCharacter(USkeletalMeshComponent* MeshComp) const;
    bool ResolveTargetYaw(const ACharacter* Character, float& OutYaw) const;
    void RotateTowardTargetYaw(ACharacter* Character, float DeltaTime) const;

    TWeakObjectPtr<ACharacter> CachedCharacter;
    bool bHadCachedOrientRotationToMovement = false;
};
