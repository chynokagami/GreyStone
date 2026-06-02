#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameplayTagContainer.h"
#include "Core/SoulsHitInterface.h"
#include "SoulsBaseCharacter.generated.h"

class UAnimMontage;
class USoundBase;
class UNiagaraSystem;
class USoulsActionSystemComponent;
class ASoulsWeaponBase;

UENUM(BlueprintType)
enum class EHitDirection : uint8
{
    Front  UMETA(DisplayName = "Front"),
    Back   UMETA(DisplayName = "Back"),
    Left   UMETA(DisplayName = "Left"),
    Right  UMETA(DisplayName = "Right")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSoulsDeath, ASoulsBaseCharacter*, DeadCharacter);

UCLASS(Abstract)
class SOULLIKE_API ASoulsBaseCharacter : public ACharacter, public ISoulsHitInterface
{
    GENERATED_BODY()

    friend class USoulsAnimNotify_DamageWindow;
    friend class ASoulsWeaponBase;

public:
    ASoulsBaseCharacter();

    UPROPERTY(BlueprintAssignable, Category = "Souls|Combat")
    FOnSoulsDeath OnDeath;

    //~ ISoulsHitInterface
    virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
    //~ End ISoulsHitInterface

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;

    /** Native listener bound to Attribute_Health. Triggers Die() on lethal crossing. */
    virtual void HandleHealthChanged(FGameplayTag AttributeTag, float NewValue, float OldValue);

    UFUNCTION(BlueprintCallable, Category = "Souls|Combat")
    virtual void Die();

    /** Spawns DefaultWeaponClass and attaches to RightHandSocket. Called from BeginPlay. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Combat")
    virtual void EquipDefaultWeapon();

    /** Bound to HitReact montage end (covers natural end + interrupt). Clears IsStunned. */
    UFUNCTION()
    void OnHitReactMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    /** Stephen's algorithm: forward�toHit + cross.Z sign. Pure, no side effects. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Combat")
    EHitDirection ComputeHitDirection(const FVector& ImpactPoint) const;

    /** Lookup HitReactMontages[Direction] and play. Binds OnMontageEnded for stun cleanup. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Combat")
    void PlayHitReactMontage(EHitDirection Direction);

    // HitReact data
    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|HitReact")
    TMap<EHitDirection, TObjectPtr<UAnimMontage>> HitReactMontages;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|HitReact")
    TObjectPtr<USoundBase> HitSound;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|HitReact")
    TObjectPtr<UNiagaraSystem> HitParticles;

    // Block defense data
    /** Fraction of incoming damage absorbed when blocking. 0.8 = 80% absorbed (20% leaks through). */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Combat|Block", meta=(ClampMin="0.0", ClampMax="1.0"))
    float BlockDamageReduction = 0.8f;

    /** Stamina drained per point of incoming damage when blocking. 1.0 = 1:1. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Combat|Block", meta=(ClampMin="0.0"))
    float BlockStaminaCostPerDamage = 1.0f;

    /** Min dot(forward, toAttacker) to count as a frontal hit. 0.5 ≈ 60° front cone. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Combat|Block", meta=(ClampMin="-1.0", ClampMax="1.0"))
    float BlockFrontConeDot = 0.5f;

    /** True if attacker is within the configured frontal cone. World-space, ignores Z. */
    UFUNCTION(BlueprintCallable, Category="Souls|Combat|Block")
    bool IsAttackerInFrontCone(const FVector& AttackerLoc) const;

    // Death data
    /** Played when Die() runs. Configure in BP_SoulsPlayerCharacter / future BP_SoulsAICharacter. */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|Death")
    TObjectPtr<UAnimMontage> DeathMontage;

    // Weapon
    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|Weapon")
    TSubclassOf<ASoulsWeaponBase> DefaultWeaponClass;

    /** Currently held weapon. Read-only at runtime. AnimNotify_DamageWindow reads this to drive Box Trace. */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Souls|Combat|Weapon")
    TObjectPtr<ASoulsWeaponBase> EquippedWeapon;

    // Team
    UPROPERTY(EditDefaultsOnly, Category = "Souls|Combat|Team", meta = (Categories = "Team"))
    FGameplayTag TeamTag;

    // Cached references
    /** Resolved in BeginPlay via FindComponentByClass. Subclasses inherit this � DO NOT re-cache in subclass. */
    UPROPERTY(Transient, VisibleInstanceOnly, Category = "Souls|Components")
    TObjectPtr<USoulsActionSystemComponent> CachedASC;
};
