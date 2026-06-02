#include "Animation/SoulsAnimNotifyState_FaceCameraYaw.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


USoulsAnimNotifyState_FaceCameraYaw::USoulsAnimNotifyState_FaceCameraYaw()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(80, 180, 255);
    bIsNativeBranchingPoint = false;
#endif
}

void USoulsAnimNotifyState_FaceCameraYaw::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                       UAnimSequenceBase* /*Animation*/,
                                                       float /*TotalDuration*/,
                                                       const FAnimNotifyEventReference& /*EventReference*/)
{
    ACharacter* Character = ResolveCharacter(MeshComp);
    CachedCharacter = Character;

    if (!Character)
    {
        return;
    }

    if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
    {
        bHadCachedOrientRotationToMovement = Move->bOrientRotationToMovement;
        if (bDisableOrientRotationToMovement)
        {
            Move->bOrientRotationToMovement = false;
        }
    }

    RotateTowardTargetYaw(Character, 0.f);
}

void USoulsAnimNotifyState_FaceCameraYaw::NotifyTick(USkeletalMeshComponent* MeshComp,
                                                     UAnimSequenceBase* /*Animation*/,
                                                     float FrameDeltaTime,
                                                     const FAnimNotifyEventReference& /*EventReference*/)
{
    ACharacter* Character = CachedCharacter.IsValid() ? CachedCharacter.Get() : ResolveCharacter(MeshComp);
    RotateTowardTargetYaw(Character, FrameDeltaTime);
}

void USoulsAnimNotifyState_FaceCameraYaw::NotifyEnd(USkeletalMeshComponent* MeshComp,
                                                     UAnimSequenceBase* /*Animation*/,
                                                     const FAnimNotifyEventReference& /*EventReference*/)
{
    ACharacter* Character = CachedCharacter.IsValid() ? CachedCharacter.Get() : ResolveCharacter(MeshComp);

    if (Character)
    {
        if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
        {
            if (bDisableOrientRotationToMovement)
            {
                Move->bOrientRotationToMovement = bHadCachedOrientRotationToMovement;
            }
        }
    }

    CachedCharacter.Reset();
    bHadCachedOrientRotationToMovement = false;
}

ACharacter* USoulsAnimNotifyState_FaceCameraYaw::ResolveCharacter(USkeletalMeshComponent* MeshComp) const
{
    if (!MeshComp)
    {
        return nullptr;
    }

    return Cast<ACharacter>(MeshComp->GetOwner());
}

bool USoulsAnimNotifyState_FaceCameraYaw::ResolveTargetYaw(const ACharacter* Character, float& OutYaw) const
{
    if (!Character)
    {
        return false;
    }

    if (bUseControllerYaw)
    {
        if (const AController* Controller = Character->GetController())
        {
            OutYaw = Controller->GetControlRotation().Yaw;
            return true;
        }
    }

    OutYaw = Character->GetActorRotation().Yaw;
    return true;
}

void USoulsAnimNotifyState_FaceCameraYaw::RotateTowardTargetYaw(ACharacter* Character, float DeltaTime) const
{
    if (!Character)
    {
        return;
    }

    float TargetYaw = 0.f;
    if (!ResolveTargetYaw(Character, TargetYaw))
    {
        return;
    }

    const FRotator CurrentRotation = Character->GetActorRotation();
    const float YawDelta = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetYaw));

    FRotator TargetRotation = CurrentRotation;
    TargetRotation.Yaw = TargetYaw;

    if (YawDelta <= SnapToleranceDegrees || DeltaTime <= 0.f || InterpSpeed <= 0.f)
    {
        if (YawDelta <= SnapToleranceDegrees)
        {
            Character->SetActorRotation(TargetRotation);
        }
        return;
    }

    const FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, InterpSpeed);
    Character->SetActorRotation(NewRotation);
}
