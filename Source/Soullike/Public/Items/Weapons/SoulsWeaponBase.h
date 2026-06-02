#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "SoulsWeaponBase.generated.h"

class UBoxComponent;
class USceneComponent;

UCLASS(Abstract, Blueprintable)
class SOULLIKE_API ASoulsWeaponBase : public AActor
{
    GENERATED_BODY()

public:
    ASoulsWeaponBase();

    UFUNCTION(BlueprintCallable, Category = "Souls|Weapon")
    void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);

    /** Opens a damage window. Called by AnimNotify_DamageWindow on NotifyBegin. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Weapon")
    void BeginHitDetection(float InDamage, FGameplayTag InDamageType);

    /** Runs one frame of continuous melee sweep. Called by DamageWindow NotifyTick. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Weapon")
    void UpdateHitDetection(float DeltaTime);

    /** Closes the damage window and clears per-swing trace state. Called on NotifyEnd. */
    UFUNCTION(BlueprintCallable, Category = "Souls|Weapon")
    void EndHitDetection();

    UFUNCTION(BlueprintPure, Category = "Souls|Weapon")
    UBoxComponent* GetWeaponBox() const { return WeaponBox; }

protected:
    virtual void BeginPlay() override;

    /** Builds evenly spaced blade samples from BoxTraceStart to BoxTraceEnd. */
    void GetTraceSamples(TArray<FVector>& OutSamples) const;

    /** Filters duplicate/team/self hits and executes damage. */
    void HandleTraceHit(const FHitResult& HitResult);

    /** Owner-vs-target team check. Returns true if same team -> skip damage. */
    bool IsSameTeam(const AActor* OtherActor) const;

    /** Apply damage + call IHitInterface on target. */
    void ExecuteHit(const FHitResult& BoxHit);

    // Components 

    /** Root component for trace markers. It is not used as an overlap trigger. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Souls|Weapon|Components")
    TObjectPtr<UBoxComponent> WeaponBox;

    /** Configured in BP; drag to blade hilt position. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Souls|Weapon|Components")
    TObjectPtr<USceneComponent> BoxTraceStart;

    /** Configured in BP; drag to blade tip position. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Souls|Weapon|Components")
    TObjectPtr<USceneComponent> BoxTraceEnd;

    // Trace settings 
    /** Half-extent of the BoxTrace shape. Narrow = matches actual blade thickness. */
    UPROPERTY(EditAnywhere, Category = "Souls|Weapon|Trace")
    FVector BoxTraceExtent = FVector(5.f);

    /** Number of blade intervals sampled each frame. 5 means 6 sweep samples along the blade. */
    UPROPERTY(EditAnywhere, Category = "Souls|Weapon|Trace", meta = (ClampMin = "1", UIMin = "1"))
    int32 TraceDensity = 5;

    /** Collision channel used by the continuous melee sweeps. */
    UPROPERTY(EditAnywhere, Category = "Souls|Weapon|Trace")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Pawn;

    /** Show debug box during trace. Toggle in BP CDO when tuning. */
    UPROPERTY(EditAnywhere, Category = "Souls|Weapon|Trace")
    bool bShowBoxDebug = false;

    // Damage state (set by AnimNotify per swing)

    /** Current swing's damage. Reset to 0 in EndHitDetection. */
    UPROPERTY(VisibleInstanceOnly, Category = "Souls|Weapon|Runtime")
    float CurrentDamage = 0.f;

    /** Current swing's damage type tag (e.g. Damage.Slash, Damage.Strike). */
    UPROPERTY(VisibleInstanceOnly, Category = "Souls|Weapon|Runtime")
    FGameplayTag CurrentDamageType;

    /** True while a DamageWindow notify is open. */
    UPROPERTY(VisibleInstanceOnly, Category = "Souls|Weapon|Runtime")
    bool bHitDetectionActive = false;

    /** Last frame's blade samples. Current tick sweeps from these positions to new positions. */
    UPROPERTY(VisibleInstanceOnly, Category = "Souls|Weapon|Runtime")
    TArray<FVector> PreviousFrameTraceSamples;

    /** Per-swing ignore list. Prevents single swing from hitting same actor twice. Cleared in EndHitDetection. */
    UPROPERTY(VisibleInstanceOnly, Category = "Souls|Weapon|Runtime")
    TArray<TObjectPtr<AActor>> IgnoredActors;
};
