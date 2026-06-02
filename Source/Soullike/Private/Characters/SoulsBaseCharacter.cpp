#include "Characters/SoulsBaseCharacter.h"

#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"           // FDamageEvent (full type for TakeDamage impl)
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"

#include "SharedGameplayTags.h"
#include "Soullike.h"  // LogGame

#include "Items/Weapons/SoulsWeaponBase.h"
#include "SoulsGameTypes.h"  // NAME_WeaponSocket_RightHand
#include "GameFramework/CharacterMovementComponent.h" 

ASoulsBaseCharacter::ASoulsBaseCharacter()
{
    // BaseCharacter is infrastructure layer � no per-frame Tick.
    PrimaryActorTick.bCanEverTick = false;

    TeamTag = SharedGameplayTags::Team_Player;

}

void ASoulsBaseCharacter::BeginPlay()
{
    Super::BeginPlay();

    CachedASC = FindComponentByClass<USoulsActionSystemComponent>();
    if (!CachedASC)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[%s] No SoulsActionSystemComponent found � add it in Blueprint"),
            *GetName());
        return;
    }

    // Native multicast bind. ~10x faster than dynamic delegate (no reflection per dispatch).
    CachedASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
        .AddUObject(this, &ThisClass::HandleHealthChanged);

    EquipDefaultWeapon();

    // ---- W5 D2: enable root-motion-from-montages on the capsule ----
    // Paragon AnimBPs ship with RootMotionMode = NoExtraction, which causes
    // a "snap back to original position" at montage end (mesh moves visually
    // but capsule doesn't follow). Forcing this from C++ is more robust than
    // hoping every AnimBP is configured correctly.
    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInst->SetRootMotionMode(ERootMotionMode::RootMotionFromMontagesOnly);
    }

}

float ASoulsBaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // Call Super first runs UE's OnTakeAnyDamage delegate, FDamageType processing, etc.
    // Super may scale DamageAmount based on damage type/team rules; we use its return value.
    const float ActualBase = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualBase <= 0.f) return 0.f;

    if (!CachedASC)
    {
        // Already warned in BeginPlay don't spam.
        return 0.f;
    }

    // Already-dead guard stops delayed projectiles or AoE residue from re-triggering Die().
    if (CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return 0.f;
    }

    if (CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IFrame))
    {
        UE_LOG(LogGame, Log, TEXT("[TakeDamage] %s nullified %.0f via IFrame"),
            *GetName(), ActualBase);
        return 0.f;
    }

    // Block path. Frontal-cone-only, stamina-gated, partial damage absorption.
    // GetHit also gates on this tag separately to skip the HitReact montage.
    if (CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsBlocking)
        && DamageCauser
        && IsAttackerInFrontCone(DamageCauser->GetActorLocation()))
    {
        // Drain stamina proportional to incoming damage.
        const float StaminaDrain = ActualBase * BlockStaminaCostPerDamage;
        CachedASC->ApplyAttributeChange(
            SharedGameplayTags::Attribute_Stamina,
            -StaminaDrain,
            EAttributeModifyType::Modifier);

        // Apply reduced damage.
        const float ReducedDamage = ActualBase * (1.f - BlockDamageReduction);
        if (ReducedDamage > 0.f)
        {
            CachedASC->ApplyAttributeChange(
                SharedGameplayTags::Attribute_Health,
                -ReducedDamage,
                EAttributeModifyType::Modifier);
        }

        // Block-hit FX (no stun, no HitReact — those are skipped in GetHit).
        if (HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
        }
        if (HitParticles)
        {
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitParticles, GetActorLocation());
        }

        // Guard break: out of stamina -> drop the block. Subsequent hits go through normal.
        if (CachedASC->GetAttributeValue(SharedGameplayTags::Attribute_Stamina) <= 0.f)
        {
            UE_LOG(LogGame, Log, TEXT("[Block] %s GUARD BROKEN (stamina depleted)"), *GetName());
            CachedASC->StopAction(SharedGameplayTags::Action_Block);
        }

        UE_LOG(LogGame, Log, TEXT("[Block] %s absorbed %.0f -> took %.0f, stamina -%.0f"),
            *GetName(), ActualBase, ReducedDamage, StaminaDrain);

        return ReducedDamage;
    }

    // D1 = Modifier mode: Health.Modifier accumulates negative deltas, Base stays at HealthMax.
    // ASC's PostAttributeChanged clamps GetValue() to [0, HealthMax], then broadcasts to listeners.
    // Our HandleHealthChanged catches the lethal crossing.
    CachedASC->ApplyAttributeChange(
        SharedGameplayTags::Attribute_Health,
        -ActualBase,
        EAttributeModifyType::Modifier);

    return ActualBase;
}

void ASoulsBaseCharacter::HandleHealthChanged(FGameplayTag /*AttributeTag*/, float NewValue, float OldValue)
{
    // Edge-trigger: only fire Die() on the lethal crossing, not every damage tick.
    if (OldValue > 0.f && NewValue <= 0.f)
    {
        Die();
    }
}

void ASoulsBaseCharacter::Die()
{
    if (!CachedASC) return;

    // Idempotent Die() may be called from multiple paths (HP=0, scripted kill, debug cmd).
    if (CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    CachedASC->ActiveGameplayTags.AddTag(SharedGameplayTags::Status_IsDead);

    // D4: lethal hit goes straight to death � DeathMontage interrupts any current HitReact.
    if (DeathMontage)
    {
        PlayAnimMontage(DeathMontage);
    }

    // Corpse non-blocking other characters / player can walk over.
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UE_LOG(LogGame, Log, TEXT("[Die] %s"), *GetName());

    // D2 = C: broadcast LAST, so subscribers see fully-dead state (tag set, montage started, collision off).
    OnDeath.Broadcast(this);
}

void ASoulsBaseCharacter::OnHitReactMontageEnded(UAnimMontage* /*Montage*/, bool /*bInterrupted*/)
{
    // Clears stun whether the montage ended naturally or got interrupted by DeathMontage.
    if (!CachedASC) return;
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsStunned);
}

void ASoulsBaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
    // Dead pawns don't react. Stops delayed projectiles from making the corpse twitch.
    if (CachedASC && CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    // Invincibility frames (set by IFrame NotifyState during a roll) — drop the hit
    // entirely: no damage, no stun, no FX. The damage call site (TakeDamage) is a
    // separate path; we'd want to short-circuit damage there too, but for W5 D2 the
    // weapon BoxTrace path goes through GetHit + ApplyDamage on the same frame, so
    // gating GetHit is enough to also signal the attacker that the hit was dodged.
    if (CachedASC && CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IFrame))
    {
        UE_LOG(LogGame, Log, TEXT("[GetHit] %s dodged via IFrame"), *GetName());
        return;
    }

    // Block guard — frontal cone check; if true, suppress stun + hit react montage.
    // Damage was already partially absorbed in TakeDamage. FX played there too.
    if (CachedASC
        && CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsBlocking)
        && Hitter
        && IsAttackerInFrontCone(Hitter->GetActorLocation()))
    {
        UE_LOG(LogGame, Log, TEXT("[GetHit] %s blocked attack from %s"),
            *GetName(), *Hitter->GetName());
        return;
    }

    // Stun tag on. PlayHitReactMontage will bind OnMontageEnded which removes it.
    if (CachedASC)
    {
        CachedASC->ActiveGameplayTags.AddTag(SharedGameplayTags::Status_IsStunned);
    }

    const EHitDirection Direction = ComputeHitDirection(ImpactPoint);
    PlayHitReactMontage(Direction);

    if (HitSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);
    }

    if (HitParticles)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), HitParticles, ImpactPoint);
    }

    UE_LOG(LogGame, Log, TEXT("[GetHit] %s hit from %s by %s"),
        *GetName(),
        *UEnum::GetValueAsString(Direction),
        Hitter ? *Hitter->GetName() : TEXT("(null)"));
}

EHitDirection ASoulsBaseCharacter::ComputeHitDirection(const FVector& ImpactPoint) const
{
    const FVector ActorLoc = GetActorLocation();
    const FVector Forward = GetActorForwardVector();
    const FVector ImpactFlat(ImpactPoint.X, ImpactPoint.Y, ActorLoc.Z);
    const FVector ToHit = (ImpactFlat - ActorLoc).GetSafeNormal();

    const double CosTheta = FVector::DotProduct(Forward, ToHit);
    double Theta = FMath::RadiansToDegrees(FMath::Acos(CosTheta));

    const FVector Cross = FVector::CrossProduct(Forward, ToHit);
    if (Cross.Z < 0.0)
    {
        Theta = -Theta;
    }

    if (Theta >= -45.f && Theta < 45.f)   return EHitDirection::Front;
    if (Theta >= 45.f && Theta < 135.f)   return EHitDirection::Right;
    if (Theta >= -135.f && Theta < -45.f)   return EHitDirection::Left;
    return EHitDirection::Back;
}

void ASoulsBaseCharacter::PlayHitReactMontage(EHitDirection Direction)
{
    const TObjectPtr<UAnimMontage>* MontagePtr = HitReactMontages.Find(Direction);
    if (!MontagePtr || !*MontagePtr)
    {
        UE_LOG(LogGame, Warning, TEXT("[HitReact] No montage for direction %s on %s"),
            *UEnum::GetValueAsString(Direction),
            *GetName());
        return;
    }

    const float Duration = PlayAnimMontage(*MontagePtr);
    if (Duration <= 0.f) return;  // PlayAnimMontage failed silently (no AnimInstance / wrong slot)

    // Bind one-shot end callback. Fires on natural end OR interrupt (e.g. DeathMontage takes over).
    if (UAnimInstance* AnimInst = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &ThisClass::OnHitReactMontageEnded);
        AnimInst->Montage_SetEndDelegate(EndDelegate, *MontagePtr);
    }
}

void ASoulsBaseCharacter::EquipDefaultWeapon()
{
    if (!DefaultWeaponClass)
    {
        UE_LOG(LogGame, Log, TEXT("[%s] No DefaultWeaponClass configured � skipping weapon equip"),
            *GetName());
        return;
    }

    if (EquippedWeapon)
    {
        UE_LOG(LogGame, Warning, TEXT("[%s] EquipDefaultWeapon called but weapon already equipped � skipping"),
            *GetName());
        return;
    }

    UWorld* World = GetWorld();
    if (!World) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.Instigator = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    EquippedWeapon = World->SpawnActor<ASoulsWeaponBase>(
        DefaultWeaponClass,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        SpawnParams);

    if (!EquippedWeapon)
    {
        UE_LOG(LogGame, Warning, TEXT("[%s] Failed to spawn weapon class %s"),
            *GetName(), *DefaultWeaponClass->GetName());
        return;
    }

    USkeletalMeshComponent* SkelMesh = GetMesh();
    if (!SkelMesh)
    {
        UE_LOG(LogGame, Warning, TEXT("[%s] No SkeletalMesh � cannot attach weapon"), *GetName());
        EquippedWeapon->Destroy();
        EquippedWeapon = nullptr;
        return;
    }

    EquippedWeapon->Equip(SkelMesh, FName(TEXT(NAME_WeaponSocket_RightHand)), this, this);

    UE_LOG(LogGame, Log, TEXT("[%s] Equipped default weapon %s to %s"),
        *GetName(), *EquippedWeapon->GetName(), TEXT(NAME_WeaponSocket_RightHand));
}

bool ASoulsBaseCharacter::IsAttackerInFrontCone(const FVector& AttackerLoc) const
{
    const FVector ToAttacker = (AttackerLoc - GetActorLocation()).GetSafeNormal2D();
    const FVector Forward = GetActorForwardVector();
    return FVector::DotProduct(Forward, ToAttacker) >= BlockFrontConeDot;
}