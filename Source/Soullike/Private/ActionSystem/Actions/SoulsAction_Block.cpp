#include "ActionSystem/Actions/SoulsAction_Block.h"

#include "Soullike.h"   // LogGame
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USoulsAction_Block::USoulsAction_Block()
{
    CooldownTime = 0.0f;
}

void USoulsAction_Block::StartAction_Implementation()
{
    Super::StartAction_Implementation();

    USoulsActionSystemComponent* ASC = GetOwningComponent();
    if (!ASC) return;

    ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
    if (!Character) return;

    // Slow movement. Save current speed so external StopAction (guard break) can
    // restore exactly without depending on Player code.
    if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
    {
        CachedMaxWalkSpeed = Move->MaxWalkSpeed;
        Move->MaxWalkSpeed = CachedMaxWalkSpeed * BlockSpeedMultiplier;
    }

    if (BlockMontage)
    {
        Character->PlayAnimMontage(BlockMontage);
    }

    UE_LOG(LogGame, Log, TEXT("[Block] START on %s"), *Character->GetName());
}

void USoulsAction_Block::StopAction_Implementation()
{
    Super::StopAction_Implementation();

    USoulsActionSystemComponent* ASC = GetOwningComponent();
    if (!ASC) return;

    ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
    if (!Character) return;

    if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
    {
        Move->MaxWalkSpeed = CachedMaxWalkSpeed;
    }

    // Stop the montage explicitly so it doesn't linger if it had a long tail.
    if (BlockMontage)
    {
        if (UAnimInstance* AnimInst = Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInst->Montage_Stop(MontageBlendOut, BlockMontage);
        }
    }

    UE_LOG(LogGame, Log, TEXT("[Block] STOP on %s"), *Character->GetName());
}