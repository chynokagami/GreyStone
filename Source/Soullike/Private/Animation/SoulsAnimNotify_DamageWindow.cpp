#include "Animation/SoulsAnimNotify_DamageWindow.h"

#include "Characters/SoulsBaseCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Items/Weapons/SoulsWeaponBase.h"

#include "Soullike.h"  // LogGame

USoulsAnimNotify_DamageWindow::USoulsAnimNotify_DamageWindow()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(50, 200, 50, 255);
#endif
}

void USoulsAnimNotify_DamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation, float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    ASoulsWeaponBase* Weapon = ResolveWeapon(MeshComp);
    if (!Weapon)
    {
        // Already logged inside ResolveWeapon; no spam here.
        return;
    }

    Weapon->BeginHitDetection(DamageAmount, DamageType);

    UE_LOG(LogGame, Verbose, TEXT("[DamageWindow] OPEN [%s] dmg=%.0f type=%s"),
        *MeshComp->GetOwner()->GetName(),
        DamageAmount,
        DamageType.IsValid() ? *DamageType.ToString() : TEXT("(none)"));
}

void USoulsAnimNotify_DamageWindow::NotifyTick(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float FrameDeltaTime,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

    ASoulsWeaponBase* Weapon = ResolveWeapon(MeshComp, false);
    if (!Weapon) return;

    Weapon->UpdateHitDetection(FrameDeltaTime);
}

void USoulsAnimNotify_DamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    ASoulsWeaponBase* Weapon = ResolveWeapon(MeshComp);
    if (!Weapon) return;

    Weapon->EndHitDetection();

    UE_LOG(LogGame, Verbose, TEXT("[DamageWindow] CLOSE [%s]"),
        *MeshComp->GetOwner()->GetName());
}

FString USoulsAnimNotify_DamageWindow::GetNotifyName_Implementation() const
{
    if (DamageType.IsValid())
    {
        return FString::Printf(TEXT("DamageWindow [%.0f / %s]"),
            DamageAmount, *DamageType.ToString());
    }
    return FString::Printf(TEXT("DamageWindow [%.0f]"), DamageAmount);
}

ASoulsWeaponBase* USoulsAnimNotify_DamageWindow::ResolveWeapon(
    USkeletalMeshComponent* MeshComp,
    bool bLogMissingWeapon) const
{
    if (!MeshComp) return nullptr;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return nullptr;

    // Animation Preview Window (anim editor scrubbing) has no real character; skip silently.
    // This prevents spammy warnings when designers tune notify timing in the montage editor.
    ASoulsBaseCharacter* Character = Cast<ASoulsBaseCharacter>(Owner);
    if (!Character) return nullptr;

    ASoulsWeaponBase* Weapon = Character->EquippedWeapon;
    if (!Weapon)
    {
        if (bLogMissingWeapon)
        {
            UE_LOG(LogGame, Warning,
                TEXT("[DamageWindow] %s has no EquippedWeapon; check DefaultWeaponClass in BP CDO"),
                *Character->GetName());
        }
        return nullptr;
    }

    return Weapon;
}
