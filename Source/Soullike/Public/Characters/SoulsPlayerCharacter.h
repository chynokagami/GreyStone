#pragma once

#include "CoreMinimal.h"
#include "Characters/SoulsBaseCharacter.h"
#include "GameplayTagContainer.h"
#include "SoulsPlayerCharacter.generated.h"


struct FInputActionValue;
class APlayerController;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UCameraComponent;
class USoulsActionSystemComponent;
class UUserWidget;


USTRUCT(BlueprintType)
struct SOULLIKE_API FHeavyChargeStage
{
    GENERATED_BODY()

    FHeavyChargeStage() = default;

    FHeavyChargeStage(float InRequiredHoldTime, float InDamageMultiplier)
        : RequiredHoldTime(InRequiredHoldTime)
        , DamageMultiplier(InDamageMultiplier)
    {
    }

    /** Seconds held before this stage becomes active. Stage 0 should usually be 0. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Souls|Heavy", meta=(ClampMin="0.0"))
    float RequiredHoldTime = 0.f;

    /** Runtime multiplier applied to DamageWindow base damage while the heavy swing is active. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Souls|Heavy", meta=(ClampMin="0.0"))
    float DamageMultiplier = 1.f;
};


UCLASS()
class SOULLIKE_API ASoulsPlayerCharacter : public ASoulsBaseCharacter
{
    GENERATED_BODY()

public:

    ASoulsPlayerCharacter();

protected:


    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USpringArmComponent> SpringArmComponent;

    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UCameraComponent> CameraComponent;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_Move;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_Look;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_LightAttack;

    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_Roll;

    // input 
    // WASD -  Vector2D(X back forward , Y left right)
    void Move(const FInputActionValue& InValue);

    // Mouse - input Vector2D(X horizon, Y vertical),turn Controller Rotation
    void Look(const FInputActionValue& InValue);

    // ASC->StartAction. Enhanced Input binding while GameplayTag for input parameter
    bool StartAction(FGameplayTag InActionName);

    // ASC->StopAction.  StartAction for(Sprint/Block) 
    void StopAction(FGameplayTag InActionName);

    // UE lifetime
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void PawnClientRestart() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void Die() override;

    void AddDefaultInputMappingContext(APlayerController* PC);
    void RemoveDeathWidgetFromViewport();
    void ResetCombatInputState();
    void RestoreRespawnAttributes();
    virtual void HandleHealthChanged(FGameplayTag AttributeTag, float NewValue, float OldValue) override;

    UPROPERTY(EditDefaultsOnly, Category="Souls|UI|Death")
    TSubclassOf<UUserWidget> DeathWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> DeathWidgetInstance;

    // Debug
	UFUNCTION(Exec, Category = "Souls|Debug")
	void DebugTakeHit(const FString& DirectionStr, float Damage = 30.f);

public:
    /** Called by SaveAttack notify (mid-attack). Opens combo input window. */
    UFUNCTION(BlueprintCallable, Category="Souls|Combo")
    void EnableComboInput();

    /** Called by ResetCombo notify (near end). Decides advance vs reset. */
    UFUNCTION(BlueprintCallable, Category="Souls|Combo")
    void TryAdvanceComboOrReset();

    /** Called when player presses attack input. Routes to either start, queue, or ignore. */
    void OnLightAttackPressed();

    /** Returns the montage to play for the current ComboIndex. nullptr if invalid. */
    UAnimMontage* GetCurrentComboMontage() const;

    /** Read-only access for UI / debug. */
    int32 GetComboIndex() const { return ComboIndex; }
    bool IsAttacking() const { return bIsAttacking; }

    UFUNCTION(BlueprintCallable, Category="Souls|Respawn")
    void RestorePlayerControlAfterRespawn();

protected:
    /** Ordered list of combo montages. Index 0 = first hit, index 3 = finisher. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Combo")
    TArray<TObjectPtr<UAnimMontage>> LightAttackMontages;

    /** Current segment index (0 .. LightAttackMontages.Num()-1). */
    UPROPERTY(VisibleInstanceOnly, Category="Souls|Combo|Debug")
    int32 ComboIndex = 0;

    /** True between SaveAttack and ResetCombo notifies (input window open). */
    UPROPERTY(VisibleInstanceOnly, Category="Souls|Combo|Debug")
    bool bComboInputOpen = false;

    /** True if player pressed attack while window was open. Consumed at ResetCombo. */
    UPROPERTY(VisibleInstanceOnly, Category="Souls|Combo|Debug")
    bool bComboQueued = false;

    /** True while a LightAttack action is running (any segment). */
    UPROPERTY(VisibleInstanceOnly, Category="Souls|Combo|Debug")
    bool bIsAttacking = false;

    /** Internal: actually start the LightAttack action via ActionSystem. */
    bool StartLightAttackAction();

public:
    /** RMB pressed: begin charging windup. */
    void OnHeavyAttackPressed();

    /** RMB released: emit light or full swing based on hold duration. */
    void OnHeavyAttackReleased();

    /** Selects tap vs charged montage based on the current charge stage. Read by Action_HeavyAttack. */
    UAnimMontage* GetCurrentHeavyMontage() const;

    bool IsHeavyCharging() const { return bIsHeavyCharging; }

    UFUNCTION(BlueprintPure, Category="Souls|Heavy")
    float GetHeavyChargePercent() const;

    UFUNCTION(BlueprintPure, Category="Souls|Heavy")
    int32 GetHeavyChargeStageIndex() const;

    UFUNCTION(BlueprintPure, Category="Souls|Heavy")
    int32 GetHeavyChargeStageCount() const;

    UFUNCTION(BlueprintPure, Category="Souls|Heavy")
    float GetHeavyChargeDamageMultiplier() const;

    UFUNCTION(BlueprintPure, Category="Souls|Heavy")
    bool IsHeavyChargeDamageActive() const;

    /** Called by the heavy attack action when the swing ends or is canceled. */
    void OnHeavyAttackActionEnded();

protected:
    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_HeavyAttack;

    /** Played on press. Should loop (or be long enough) so it covers the full hold. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy")
    TObjectPtr<UAnimMontage> HeavyChargeMontage;

    /** New staged charge intro. Plays once before entering the current loop stage. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation")
    TObjectPtr<UAnimMontage> HeavyChargeInMontage;

    /** Effective IN duration before entering LOOP1. 0 uses the montage's real length. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation", meta=(ClampMin="0.0", Units="s"))
    float HeavyChargeInPlayTime = 0.f;

    /** When true, IN/LOOP play times drive stage changes instead of HeavyChargeStages.RequiredHoldTime. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation")
    bool bUseHeavyChargeAnimationTiming = true;

    /** Loop stage montages. Index 0/1/2 map to LOOP1/LOOP2/LOOP3. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation")
    TArray<TObjectPtr<UAnimMontage>> HeavyChargeLoopMontages;

    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation", meta=(ClampMin="0.0"))
    TArray<float> HeavyChargeLoopPlayTimes;

    /** Release montage used from LOOP1. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation")
    TObjectPtr<UAnimMontage> HeavyChargeOut1Montage;

    /** Release montage used from LOOP2 or LOOP3. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy|Animation")
    TObjectPtr<UAnimMontage> HeavyChargeOut2Montage;

    /** Played on early release (light tap). Lower damage. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy")
    TObjectPtr<UAnimMontage> HeavyAttackLightMontage;

    /** Played on full charge release. Higher damage. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy")
    TObjectPtr<UAnimMontage> HeavyAttackFullMontage;

    /** Hold duration to reach "full charge" tier when HeavyChargeStages is empty. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy", meta=(ClampMin="0.05"))
    float HeavyFullChargeThreshold = 0.5f;

    /** Ordered tuning data for charge stages. Highest reached RequiredHoldTime wins. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy")
    TArray<FHeavyChargeStage> HeavyChargeStages;

    /** Walk speed multiplier while charging. 0.4 = 40% normal speed. */
    UPROPERTY(EditDefaultsOnly, Category="Souls|Heavy", meta=(ClampMin="0.0", ClampMax="1.0"))
    float HeavyChargeSpeedMultiplier = 0.4f;

    UPROPERTY(VisibleInstanceOnly, Category="Souls|Heavy|Debug")
    bool bIsHeavyCharging = false;

    UPROPERTY(VisibleInstanceOnly, Category="Souls|Heavy|Debug")
    bool bHeavyChargedFull = false;

    UPROPERTY(VisibleInstanceOnly, Category="Souls|Heavy|Debug")
    int32 HeavyChargeStageIndex = 0;

    UPROPERTY(VisibleInstanceOnly, Category="Souls|Heavy|Debug")
    float HeavyChargeDamageMultiplier = 1.f;

    UPROPERTY(VisibleInstanceOnly, Category="Souls|Heavy|Debug")
    bool bApplyHeavyChargeDamage = false;

    /** Internal: world time at press, for measuring charge duration. */
    float HeavyChargeStartTime = 0.f;

    /** Internal: original MaxWalkSpeed before slowdown, restored on release. */
    float HeavyOriginalMaxWalkSpeed = 0.f;

    bool bHeavyMovementSlowed = false;
    bool bHeavyChargeIntroPlaying = false;
    float HeavyChargeIntroEndTime = 0.f;
    int32 ActiveHeavyChargeLoopIndex = INDEX_NONE;
    float ActiveHeavyChargeLoopStartTime = 0.f;

    float GetHeavyChargeMaxHoldTime() const;
    int32 GetTopHeavyChargeStageIndex() const;
    float GetHeavyChargeStageEntryTime(int32 StageIndex) const;
    int32 GetHeavyChargeStageForDuration(float HoldDuration) const;
    float GetHeavyChargeMultiplierForStage(int32 StageIndex) const;
    bool UsesStagedHeavyChargeAnimation() const;
    float GetHeavyChargeInEffectivePlayTime() const;
    float GetHeavyChargeLoopEffectivePlayTime(int32 LoopIndex) const;
    int32 GetHeavyChargeLoopIndexForStage(int32 StageIndex) const;
    UAnimMontage* GetHeavyChargeLoopMontageForStage(int32 StageIndex) const;
    UAnimMontage* GetHeavyChargeOutMontageForStage(int32 StageIndex) const;
    void UpdateHeavyChargeState(float HoldDuration, bool bUpdateHUD);
    void UpdateHeavyChargeHUD(bool bVisible) const;
    void UpdateHeavyChargeAnimation();
    void PlayHeavyChargeLoopMontage(bool bForceReplay);
    void StopHeavyChargeAnimation();
    void RestoreHeavyChargeMovement();
    void ResetHeavyChargeDamageState();
    void CancelHeavyCharge();

public:
    /** Spacebar pressed. Cancels current attack/charge and starts the roll action. */
    void OnRollPressed();

    /** World-space roll direction. Reads cached WASD input; falls back to actor-back. */
    FVector GetRollDirectionWS() const;

protected:
    /** Updated every frame Move() is invoked; consumed by GetRollDirectionWS(). */
    UPROPERTY(VisibleInstanceOnly, Category="Souls|Roll|Debug")
    FVector2D LastMoveInput = FVector2D::ZeroVector;

public:
    void OnBlockPressed();
    void OnBlockReleased();

protected:
    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputAction> Input_Block;

};
