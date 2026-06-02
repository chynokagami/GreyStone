#include "Characters/SoulsPlayerCharacter.h"

#include "Soullike.h"                    // LogGame
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimInstance.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "SharedGameplayTags.h"
#include "Core/SoulsHitInterface.h"
#include "Blueprint/UserWidget.h"
#include "Game/SoulsUIPlayerController.h"
#include "GameFramework/PlayerController.h"
#include "HUD/SoulsHUD.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "Components/CapsuleComponent.h"

namespace
{
    TWeakObjectPtr<UUserWidget> GActiveDeathWidget;
}

ASoulsPlayerCharacter::ASoulsPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    HeavyChargeStages = {
        FHeavyChargeStage(0.0f, 1.0f),
        FHeavyChargeStage(0.2f, 1.25f),
        FHeavyChargeStage(0.65f, 1.6f),
        FHeavyChargeStage(1.1f, 2.1f)
    };
    HeavyChargeInPlayTime = 0.2f;
    HeavyChargeLoopPlayTimes = {0.45f, 0.45f, 0.45f};

    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComponent->SetupAttachment(RootComponent);
    SpringArmComponent->bUsePawnControlRotation = true;

    SpringArmComponent->TargetArmLength = 500.f;
    SpringArmComponent->SocketOffset = FVector(0, 50, 0);
    SpringArmComponent->bDoCollisionTest = true;


    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComponent->SetupAttachment(SpringArmComponent);
    CameraComponent->bUsePawnControlRotation = false;

    bUseControllerRotationYaw = false;

    // character face to the camera (WASD auto turn around)
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
}

void ASoulsPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        AddDefaultInputMappingContext(PC);
    }

}

void ASoulsPlayerCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsHeavyCharging)
    {
        const float HoldDuration = GetWorld() ? GetWorld()->TimeSeconds - HeavyChargeStartTime : 0.f;
        UpdateHeavyChargeState(HoldDuration, true);
    }
}

void ASoulsPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    RestorePlayerControlAfterRespawn();
}

void ASoulsPlayerCharacter::PawnClientRestart()
{
    Super::PawnClientRestart();

    RestorePlayerControlAfterRespawn();
}

void ASoulsPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EnhancedInput) return;

    // Move / Look bind
    if (Input_Move)
    {
        EnhancedInput->BindAction(Input_Move, ETriggerEvent::Triggered, this, &ThisClass::Move);
    }

    if (Input_Look)
    {
        EnhancedInput->BindAction(Input_Look, ETriggerEvent::Triggered, this, &ThisClass::Look);
    }

    if (Input_LightAttack)
    {
        // Route through OnLightAttackPressed so the combo state machine decides
        // whether to start, queue, or ignore the press.
        EnhancedInput->BindAction(Input_LightAttack, ETriggerEvent::Started, this,
            &ThisClass::OnLightAttackPressed);
    }

    if (Input_HeavyAttack)
    {
        // Started fires once on press; Completed fires once on release. No triggers needed.
        EnhancedInput->BindAction(Input_HeavyAttack, ETriggerEvent::Started, this,
            &ThisClass::OnHeavyAttackPressed);

        EnhancedInput->BindAction(Input_HeavyAttack, ETriggerEvent::Completed, this,
            &ThisClass::OnHeavyAttackReleased);
    }

    if (Input_Roll)
    {
        EnhancedInput->BindAction(Input_Roll, ETriggerEvent::Started, this,
            &ThisClass::OnRollPressed);
    }

        if (Input_Block)
    {
        EnhancedInput->BindAction(Input_Block, ETriggerEvent::Started, this,
            &ThisClass::OnBlockPressed);
        EnhancedInput->BindAction(Input_Block, ETriggerEvent::Completed, this,
            &ThisClass::OnBlockReleased);
    }

}

void ASoulsPlayerCharacter::Die()
{
    Super::Die();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    if (ASoulsUIPlayerController* SoulsPC = Cast<ASoulsUIPlayerController>(PC))
    {
        SoulsPC->ShowDeathScreen();
        return;
    }

    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    PC->bShowMouseCursor = true;
    PC->bEnableClickEvents = true;
    PC->bEnableMouseOverEvents = true;

    if (!DeathWidgetInstance && DeathWidgetClass)
    {
        DeathWidgetInstance = CreateWidget<UUserWidget>(PC, DeathWidgetClass);
        if (DeathWidgetInstance)
        {
            DeathWidgetInstance->AddToViewport(100);
            GActiveDeathWidget = DeathWidgetInstance;
        }
    }

    PC->SetInputMode(FInputModeUIOnly());
}

void ASoulsPlayerCharacter::HandleHealthChanged(FGameplayTag AttributeTag, float NewValue, float OldValue)
{
    Super::HandleHealthChanged(AttributeTag, NewValue, OldValue);

    if (OldValue <= 0.f && NewValue > 0.f)
    {
        RestorePlayerControlAfterRespawn();
    }
}

void ASoulsPlayerCharacter::AddDefaultInputMappingContext(APlayerController* PC)
{
    if (!PC || !DefaultMappingContext)
    {
        return;
    }

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    if (!LocalPlayer)
    {
        return;
    }

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
    {
        Subsystem->RemoveMappingContext(DefaultMappingContext);
        Subsystem->AddMappingContext(DefaultMappingContext, 0);
    }
}

void ASoulsPlayerCharacter::RemoveDeathWidgetFromViewport()
{
    if (DeathWidgetInstance)
    {
        DeathWidgetInstance->RemoveFromParent();
        DeathWidgetInstance = nullptr;
    }

    if (GActiveDeathWidget.IsValid())
    {
        GActiveDeathWidget->RemoveFromParent();
        GActiveDeathWidget.Reset();
    }
}

void ASoulsPlayerCharacter::ResetCombatInputState()
{
    bComboInputOpen = false;
    bComboQueued = false;
    bIsAttacking = false;
    ComboIndex = 0;

    RestoreHeavyChargeMovement();
    bIsHeavyCharging = false;
    bHeavyChargedFull = false;
    HeavyChargeStageIndex = 0;
    HeavyChargeDamageMultiplier = 1.f;
    bApplyHeavyChargeDamage = false;
    HeavyChargeStartTime = 0.f;
    HeavyOriginalMaxWalkSpeed = 0.f;
    bHeavyMovementSlowed = false;
    bHeavyChargeIntroPlaying = false;
    HeavyChargeIntroEndTime = 0.f;
    ActiveHeavyChargeLoopIndex = INDEX_NONE;
    ActiveHeavyChargeLoopStartTime = 0.f;
    UpdateHeavyChargeHUD(false);

    LastMoveInput = FVector2D::ZeroVector;
}

void ASoulsPlayerCharacter::RestoreRespawnAttributes()
{
    if (!CachedASC)
    {
        return;
    }

    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsDead);
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsAttacking);
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsRolling);
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsBlocking);
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IsStunned);
    CachedASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IFrame);

    const float HealthMax = CachedASC->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);
    const float Health = CachedASC->GetAttributeValue(SharedGameplayTags::Attribute_Health);
    if (HealthMax > 0.f && Health <= 0.f)
    {
        CachedASC->ApplyAttributeChange(
            SharedGameplayTags::Attribute_Health,
            HealthMax - Health,
            EAttributeModifyType::Modifier);
    }

    const float StaminaMax = CachedASC->GetAttributeValue(SharedGameplayTags::Attribute_StaminaMax);
    const float Stamina = CachedASC->GetAttributeValue(SharedGameplayTags::Attribute_Stamina);
    if (StaminaMax > 0.f && Stamina <= 0.f)
    {
        CachedASC->ApplyAttributeChange(
            SharedGameplayTags::Attribute_Stamina,
            StaminaMax - Stamina,
            EAttributeModifyType::Modifier);
    }
}

void ASoulsPlayerCharacter::RestorePlayerControlAfterRespawn()
{
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    AddDefaultInputMappingContext(PC);
    RemoveDeathWidgetFromViewport();
    ResetCombatInputState();
    RestoreRespawnAttributes();

    PC->ResetIgnoreMoveInput();
    PC->ResetIgnoreLookInput();
    PC->bShowMouseCursor = false;
    PC->bEnableClickEvents = false;
    PC->bEnableMouseOverEvents = false;
    PC->SetInputMode(FInputModeGameOnly());

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->SetMovementMode(MOVE_Walking);
    }
}

void ASoulsPlayerCharacter::Move(const FInputActionValue& InValue)
{
    const FVector2D InputValue = InValue.Get<FVector2D>();

    // Cache for OnRollPressed -> roll direction lookup. Doesn't decay; whatever was
    // pressed last is what we use, even after release. That's intentional: in Souls
    // the dodge direction is "the way you were moving", and a player who wants to
    // dodge backward simply releases keys before pressing space.
    LastMoveInput = InputValue;

    FRotator ControlRot = GetControlRotation();
    ControlRot.Pitch = 0.0f;
    ControlRot.Roll = 0.0f;

    AddMovementInput(ControlRot.Vector(), InputValue.X);

    const FVector RightDirection = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);
    AddMovementInput(RightDirection, InputValue.Y);
}

void ASoulsPlayerCharacter::Look(const FInputActionValue& InValue)
{
    const FVector2D InputValue = InValue.Get<FVector2D>();

    // X (Yaw), Y (Pitch)
    AddControllerYawInput(InputValue.X);
    AddControllerPitchInput(InputValue.Y);
}

bool ASoulsPlayerCharacter::StartAction(FGameplayTag InActionName)
{
    if (CachedASC)
    {
        return CachedASC->StartAction(InActionName);
    }
    return false;
}

void ASoulsPlayerCharacter::StopAction(FGameplayTag InActionName)
{
    if (CachedASC)
    {
        CachedASC->StopAction(InActionName);
    }
}

// ===== Debug =====

void ASoulsPlayerCharacter::DebugTakeHit(const FString& DirectionStr, float Damage)
{
    const FVector ActorLoc = GetActorLocation();
    const FVector Forward = GetActorForwardVector();
    const FVector Right = GetActorRightVector();
    constexpr float Offset = 100.f;

    FVector ImpactPoint;

    if (DirectionStr.Equals(TEXT("Front"), ESearchCase::IgnoreCase)) ImpactPoint = ActorLoc + Forward * Offset;
    else if (DirectionStr.Equals(TEXT("Back"), ESearchCase::IgnoreCase)) ImpactPoint = ActorLoc - Forward * Offset;
    else if (DirectionStr.Equals(TEXT("Left"), ESearchCase::IgnoreCase)) ImpactPoint = ActorLoc - Right * Offset;
    else if (DirectionStr.Equals(TEXT("Right"), ESearchCase::IgnoreCase)) ImpactPoint = ActorLoc + Right * Offset;
    else
    {
        UE_LOG(LogGame, Warning,
            TEXT("[DebugTakeHit] Unknown direction '%s'. Use: Front | Back | Left | Right"),
            *DirectionStr);
        return;
    }

    // 1) Visual reaction first — adds Stun, plays HitReact montage.
    ISoulsHitInterface::Execute_GetHit(this, ImpactPoint, this);

    // 2) Damage — may trigger Die(), whose DeathMontage interrupts HitReact.
    //    Order matters: HitReact starts, then Death interrupts it. Inverse order = Death starts, then HitReact takes over (wrong).
    if (Damage > 0.f)
    {
        UGameplayStatics::ApplyDamage(this, Damage, GetController(), this, UDamageType::StaticClass());
    }
}


void ASoulsPlayerCharacter::OnLightAttackPressed()
{
    // Dead-state guard. The "death" state lives in CachedASC's tag container
    // (Status.IsDead added by ASoulsBaseCharacter::Die), not as a bool member.
    if (!CachedASC || CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    // Don't break a heavy charge with a stray LMB press.
    if (bIsHeavyCharging)
    {
        return;
    }

    // Cancel block — pressing attack drops the shield to swing.
    CachedASC->StopAction(SharedGameplayTags::Action_Block);

    if (!bIsAttacking)
    {
        // First press: start chain from segment 0.
        ComboIndex = 0;
        StartLightAttackAction();
        return;
    }

    // Already attacking. Queue next press only if the input window is open.
    if (bComboInputOpen)
    {
        bComboQueued = true;
        UE_LOG(LogGame, Log, TEXT("[Combo] Queued next attack (current idx=%d)"), ComboIndex);
    }
    // Else: ignored (too early or too late in the swing).
}

void ASoulsPlayerCharacter::EnableComboInput()
{
    bComboInputOpen = true;
    UE_LOG(LogGame, Verbose, TEXT("[Combo] Input window OPEN at idx=%d"), ComboIndex);
}

void ASoulsPlayerCharacter::TryAdvanceComboOrReset()
{
    bComboInputOpen = false;

    if (!bComboQueued)
    {
        // No buffered input -> chain ends here.
        ComboIndex = 0;
        bIsAttacking = false;
        UE_LOG(LogGame, Log, TEXT("[Combo] Reset (no input queued)"));
        return;
    }

    // Buffered input -> advance.
    bComboQueued = false;
    const int32 NumSegments = LightAttackMontages.Num();
    if (NumSegments <= 0)
    {
        ComboIndex = 0;
        bIsAttacking = false;
        return;
    }

    ComboIndex = (ComboIndex + 1) % NumSegments;
    UE_LOG(LogGame, Log, TEXT("[Combo] Advance to idx=%d"), ComboIndex);

    StartLightAttackAction();
}

UAnimMontage* ASoulsPlayerCharacter::GetCurrentComboMontage() const
{
    if (!LightAttackMontages.IsValidIndex(ComboIndex))
    {
        return nullptr;
    }
    return LightAttackMontages[ComboIndex];
}

bool ASoulsPlayerCharacter::StartLightAttackAction()
{
    if (!CachedASC) return false;

    // Force-stop any previous LightAttack instance so CanStart() (which rejects
    // already-running actions) doesn't block our combo-advance restart.
    // No-op if not currently running.
    CachedASC->StopAction(SharedGameplayTags::Action_LightAttack);

    const bool bStarted = CachedASC->StartAction(SharedGameplayTags::Action_LightAttack);
    if (!bStarted)
    {
        bIsAttacking = false;
        bComboInputOpen = false;
        bComboQueued = false;
        ComboIndex = 0;
        return false;
    }

    bIsAttacking = true;
    return true;
}


void ASoulsPlayerCharacter::OnHeavyAttackPressed()
{
    if (!CachedASC || CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    // Block re-entry: don't allow heavy mid-light-attack or double-press during charge.
    if (bIsAttacking || bIsHeavyCharging)
    {
        return;
    }

    CachedASC->StopAction(SharedGameplayTags::Action_Block);

    bIsHeavyCharging = true;
    bHeavyChargedFull = false;
    HeavyChargeStageIndex = 0;
    HeavyChargeDamageMultiplier = 1.f;
    bApplyHeavyChargeDamage = false;
    HeavyChargeStartTime = GetWorld()->TimeSeconds;
    bHeavyChargeIntroPlaying = false;
    HeavyChargeIntroEndTime = 0.f;
    ActiveHeavyChargeLoopIndex = INDEX_NONE;
    ActiveHeavyChargeLoopStartTime = 0.f;

    // Slow movement. Save current MaxWalkSpeed so we restore exactly (in case it
    // was modified elsewhere — e.g. exhausted state — between press and release).
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        HeavyOriginalMaxWalkSpeed = Move->MaxWalkSpeed;
        Move->MaxWalkSpeed = HeavyOriginalMaxWalkSpeed * HeavyChargeSpeedMultiplier;
        bHeavyMovementSlowed = true;
    }

    // Play the staged charge intro first. Tick advances into LOOP1/2/3.
    if (HeavyChargeInMontage)
    {
        const float IntroDuration = PlayAnimMontage(HeavyChargeInMontage);
        if (IntroDuration > 0.f)
        {
            const float EffectiveIntroDuration = GetHeavyChargeInEffectivePlayTime();
            bHeavyChargeIntroPlaying = true;
            HeavyChargeIntroEndTime = HeavyChargeStartTime + EffectiveIntroDuration;
        }
        else
        {
            PlayHeavyChargeLoopMontage(true);
        }
    }
    else if (HeavyChargeLoopMontages.Num() > 0)
    {
        PlayHeavyChargeLoopMontage(true);
    }
    else if (HeavyChargeMontage)
    {
        PlayAnimMontage(HeavyChargeMontage);
    }

    UpdateHeavyChargeHUD(true);

    UE_LOG(LogGame, Log, TEXT("[Heavy] Charge START at %.3f"), HeavyChargeStartTime);
}

void ASoulsPlayerCharacter::OnHeavyAttackReleased()
{
    // Safe no-op for orphan release (focus loss, etc.)
    if (!bIsHeavyCharging)
    {
        return;
    }

    bIsHeavyCharging = false;
    RestoreHeavyChargeMovement();

    const float HoldDuration = GetWorld()->TimeSeconds - HeavyChargeStartTime;
    UpdateHeavyChargeState(HoldDuration, true);
    UpdateHeavyChargeHUD(false);
    bHeavyChargedFull = HeavyChargeStageIndex >= GetTopHeavyChargeStageIndex();

    UE_LOG(LogGame, Log, TEXT("[Heavy] Released after %.2fs (stage=%d multiplier=%.2f fullCharge=%d)"),
        HoldDuration, HeavyChargeStageIndex, HeavyChargeDamageMultiplier, bHeavyChargedFull ? 1 : 0);

    // Same Stop+Start dance as combo restart. Defensive cooldown guard in CanStart
    // makes this safe even when an earlier heavy is still ticking down.
    if (CachedASC)
    {
        CachedASC->StopAction(SharedGameplayTags::Action_HeavyAttack);
        UpdateHeavyChargeState(HoldDuration, false);
        bApplyHeavyChargeDamage = true;
        const bool bStarted = CachedASC->StartAction(SharedGameplayTags::Action_HeavyAttack);
        if (bStarted)
        {
            bIsAttacking = true;
            return;
        }
    }

    StopHeavyChargeAnimation();
    bIsAttacking = false;
    ResetHeavyChargeDamageState();
}

UAnimMontage* ASoulsPlayerCharacter::GetCurrentHeavyMontage() const
{
    if (UAnimMontage* OutMontage = GetHeavyChargeOutMontageForStage(HeavyChargeStageIndex))
    {
        return OutMontage;
    }

    return HeavyChargeStageIndex > 0 ? HeavyAttackFullMontage : HeavyAttackLightMontage;
}

float ASoulsPlayerCharacter::GetHeavyChargePercent() const
{
    if (!bIsHeavyCharging || !GetWorld())
    {
        return 0.f;
    }

    const float MaxHoldTime = GetHeavyChargeMaxHoldTime();
    if (MaxHoldTime <= KINDA_SMALL_NUMBER)
    {
        return 1.f;
    }

    return FMath::Clamp((GetWorld()->TimeSeconds - HeavyChargeStartTime) / MaxHoldTime, 0.f, 1.f);
}

int32 ASoulsPlayerCharacter::GetHeavyChargeStageCount() const
{
    return HeavyChargeStages.Num() > 0 ? HeavyChargeStages.Num() : 2;
}

int32 ASoulsPlayerCharacter::GetHeavyChargeStageIndex() const
{
    return HeavyChargeStageIndex;
}

float ASoulsPlayerCharacter::GetHeavyChargeDamageMultiplier() const
{
    return HeavyChargeDamageMultiplier;
}

bool ASoulsPlayerCharacter::IsHeavyChargeDamageActive() const
{
    return bApplyHeavyChargeDamage;
}

void ASoulsPlayerCharacter::OnHeavyAttackActionEnded()
{
    bIsAttacking = false;
    ResetHeavyChargeDamageState();
    bHeavyChargedFull = false;
    HeavyChargeStageIndex = 0;
    ActiveHeavyChargeLoopIndex = INDEX_NONE;
    ActiveHeavyChargeLoopStartTime = 0.f;
}

float ASoulsPlayerCharacter::GetHeavyChargeMaxHoldTime() const
{
    if (bUseHeavyChargeAnimationTiming && UsesStagedHeavyChargeAnimation())
    {
        return GetHeavyChargeStageEntryTime(GetTopHeavyChargeStageIndex());
    }

    float MaxHoldTime = HeavyFullChargeThreshold;
    for (const FHeavyChargeStage& Stage : HeavyChargeStages)
    {
        MaxHoldTime = FMath::Max(MaxHoldTime, Stage.RequiredHoldTime);
    }
    return MaxHoldTime;
}

int32 ASoulsPlayerCharacter::GetTopHeavyChargeStageIndex() const
{
    return HeavyChargeStages.Num() > 0 ? HeavyChargeStages.Num() - 1 : 1;
}

float ASoulsPlayerCharacter::GetHeavyChargeStageEntryTime(int32 StageIndex) const
{
    if (StageIndex <= 0)
    {
        return 0.f;
    }

    if (!bUseHeavyChargeAnimationTiming || !UsesStagedHeavyChargeAnimation())
    {
        return HeavyChargeStages.IsValidIndex(StageIndex)
            ? HeavyChargeStages[StageIndex].RequiredHoldTime
            : HeavyFullChargeThreshold;
    }

    float EntryTime = GetHeavyChargeInEffectivePlayTime();
    for (int32 LoopIndex = 0; LoopIndex < StageIndex - 1; ++LoopIndex)
    {
        EntryTime += GetHeavyChargeLoopEffectivePlayTime(LoopIndex);
    }

    return EntryTime;
}

int32 ASoulsPlayerCharacter::GetHeavyChargeStageForDuration(float HoldDuration) const
{
    if (HeavyChargeStages.Num() <= 0)
    {
        return HoldDuration >= HeavyFullChargeThreshold ? 1 : 0;
    }

    int32 BestStageIndex = 0;
    float BestRequiredTime = -1.f;
    for (int32 Index = 0; Index < HeavyChargeStages.Num(); ++Index)
    {
        const float RequiredTime = GetHeavyChargeStageEntryTime(Index);
        if (HoldDuration >= RequiredTime && RequiredTime >= BestRequiredTime)
        {
            BestStageIndex = Index;
            BestRequiredTime = RequiredTime;
        }
    }

    return BestStageIndex;
}

float ASoulsPlayerCharacter::GetHeavyChargeMultiplierForStage(int32 StageIndex) const
{
    if (HeavyChargeStages.IsValidIndex(StageIndex))
    {
        return HeavyChargeStages[StageIndex].DamageMultiplier;
    }

    return StageIndex > 0 ? 2.f : 1.f;
}

bool ASoulsPlayerCharacter::UsesStagedHeavyChargeAnimation() const
{
    return HeavyChargeInMontage || HeavyChargeLoopMontages.Num() > 0 || HeavyChargeOut1Montage || HeavyChargeOut2Montage;
}

float ASoulsPlayerCharacter::GetHeavyChargeInEffectivePlayTime() const
{
    if (HeavyChargeInPlayTime > KINDA_SMALL_NUMBER)
    {
        return HeavyChargeInPlayTime;
    }

    return HeavyChargeInMontage ? HeavyChargeInMontage->GetPlayLength() : 0.f;
}

float ASoulsPlayerCharacter::GetHeavyChargeLoopEffectivePlayTime(int32 LoopIndex) const
{
    if (HeavyChargeLoopPlayTimes.IsValidIndex(LoopIndex)
        && HeavyChargeLoopPlayTimes[LoopIndex] > KINDA_SMALL_NUMBER)
    {
        return HeavyChargeLoopPlayTimes[LoopIndex];
    }

    if (HeavyChargeLoopMontages.IsValidIndex(LoopIndex) && HeavyChargeLoopMontages[LoopIndex])
    {
        return HeavyChargeLoopMontages[LoopIndex]->GetPlayLength();
    }

    return 0.f;
}

int32 ASoulsPlayerCharacter::GetHeavyChargeLoopIndexForStage(int32 StageIndex) const
{
    if (HeavyChargeLoopMontages.Num() <= 0)
    {
        return INDEX_NONE;
    }

    // Stage 0 is tap/uncharged. LOOP1 begins at charge stage 1.
    const int32 LoopIndex = FMath::Max(0, StageIndex - 1);
    return FMath::Clamp(LoopIndex, 0, HeavyChargeLoopMontages.Num() - 1);
}

UAnimMontage* ASoulsPlayerCharacter::GetHeavyChargeLoopMontageForStage(int32 StageIndex) const
{
    const int32 LoopIndex = GetHeavyChargeLoopIndexForStage(StageIndex);
    return HeavyChargeLoopMontages.IsValidIndex(LoopIndex) ? HeavyChargeLoopMontages[LoopIndex] : nullptr;
}

UAnimMontage* ASoulsPlayerCharacter::GetHeavyChargeOutMontageForStage(int32 StageIndex) const
{
    if (StageIndex <= 1)
    {
        return HeavyChargeOut1Montage;
    }

    return HeavyChargeOut2Montage ? HeavyChargeOut2Montage : HeavyChargeOut1Montage;
}

void ASoulsPlayerCharacter::UpdateHeavyChargeState(float HoldDuration, bool bUpdateHUD)
{
    HeavyChargeStageIndex = GetHeavyChargeStageForDuration(FMath::Max(0.f, HoldDuration));
    HeavyChargeDamageMultiplier = GetHeavyChargeMultiplierForStage(HeavyChargeStageIndex);
    bHeavyChargedFull = HeavyChargeStageIndex >= GetTopHeavyChargeStageIndex();

    UpdateHeavyChargeAnimation();

    if (bUpdateHUD)
    {
        UpdateHeavyChargeHUD(bIsHeavyCharging);
    }
}

void ASoulsPlayerCharacter::UpdateHeavyChargeHUD(bool bVisible) const
{
    const APlayerController* PC = Cast<APlayerController>(GetController());
    ASoulsHUD* SoulsHUD = PC ? Cast<ASoulsHUD>(PC->GetHUD()) : nullptr;
    if (!SoulsHUD)
    {
        return;
    }

    SoulsHUD->SetHeavyChargeVisible(bVisible);
    SoulsHUD->SetHeavyChargePercent(bVisible ? GetHeavyChargePercent() : 0.f);
    SoulsHUD->SetHeavyChargeStage(HeavyChargeStageIndex, FMath::Max(1, GetHeavyChargeStageCount() - 1));
}

void ASoulsPlayerCharacter::UpdateHeavyChargeAnimation()
{
    if (!bIsHeavyCharging || !GetWorld() || !UsesStagedHeavyChargeAnimation())
    {
        return;
    }

    if (bHeavyChargeIntroPlaying)
    {
        if (GetWorld()->TimeSeconds < HeavyChargeIntroEndTime)
        {
            return;
        }

        bHeavyChargeIntroPlaying = false;
    }

    PlayHeavyChargeLoopMontage(false);
}

void ASoulsPlayerCharacter::PlayHeavyChargeLoopMontage(bool bForceReplay)
{
    const int32 DesiredLoopIndex = GetHeavyChargeLoopIndexForStage(HeavyChargeStageIndex);
    if (DesiredLoopIndex == INDEX_NONE)
    {
        return;
    }

    if (!bForceReplay && ActiveHeavyChargeLoopIndex == DesiredLoopIndex)
    {
        UAnimMontage* CurrentLoopMontage = GetHeavyChargeLoopMontageForStage(HeavyChargeStageIndex);
        const UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr;
        const float LoopPlayTime = GetHeavyChargeLoopEffectivePlayTime(DesiredLoopIndex);
        const bool bReachedConfiguredPlayTime = LoopPlayTime > KINDA_SMALL_NUMBER
            && GetWorld()
            && (GetWorld()->TimeSeconds - ActiveHeavyChargeLoopStartTime) >= LoopPlayTime;

        if (DesiredLoopIndex == HeavyChargeLoopMontages.Num() - 1 && bReachedConfiguredPlayTime)
        {
            OnHeavyAttackReleased();
            return;
        }

        const bool bWithinConfiguredPlayTime = !bReachedConfiguredPlayTime;
        if (CurrentLoopMontage && AnimInstance && AnimInstance->Montage_IsPlaying(CurrentLoopMontage) && bWithinConfiguredPlayTime)
        {
            return;
        }
    }

    UAnimMontage* LoopMontage = GetHeavyChargeLoopMontageForStage(HeavyChargeStageIndex);
    if (!LoopMontage)
    {
        return;
    }

    PlayAnimMontage(LoopMontage);
    ActiveHeavyChargeLoopIndex = DesiredLoopIndex;
    ActiveHeavyChargeLoopStartTime = GetWorld() ? GetWorld()->TimeSeconds : 0.f;
}

void ASoulsPlayerCharacter::StopHeavyChargeAnimation()
{
    if (HeavyChargeInMontage)
    {
        StopAnimMontage(HeavyChargeInMontage);
    }

    if (HeavyChargeMontage)
    {
        StopAnimMontage(HeavyChargeMontage);
    }

    for (TObjectPtr<UAnimMontage> LoopMontage : HeavyChargeLoopMontages)
    {
        if (LoopMontage)
        {
            StopAnimMontage(LoopMontage);
        }
    }

    bHeavyChargeIntroPlaying = false;
    HeavyChargeIntroEndTime = 0.f;
    ActiveHeavyChargeLoopIndex = INDEX_NONE;
    ActiveHeavyChargeLoopStartTime = 0.f;
}

void ASoulsPlayerCharacter::RestoreHeavyChargeMovement()
{
    if (!bHeavyMovementSlowed)
    {
        return;
    }

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed = HeavyOriginalMaxWalkSpeed;
    }

    bHeavyMovementSlowed = false;
    HeavyOriginalMaxWalkSpeed = 0.f;
}

void ASoulsPlayerCharacter::ResetHeavyChargeDamageState()
{
    bApplyHeavyChargeDamage = false;
    HeavyChargeDamageMultiplier = 1.f;
}

void ASoulsPlayerCharacter::CancelHeavyCharge()
{
    bIsHeavyCharging = false;
    bHeavyChargedFull = false;
    HeavyChargeStageIndex = 0;
    HeavyChargeStartTime = 0.f;
    StopHeavyChargeAnimation();
    RestoreHeavyChargeMovement();
    ResetHeavyChargeDamageState();
    UpdateHeavyChargeHUD(false);
}


void ASoulsPlayerCharacter::OnRollPressed()
{
    if (!CachedASC || CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    // ----- Cancel any in-progress combat actions before rolling -----
    // Soulslike convention: roll has near-universal cancel priority.
    // Order: stop the action(s), reset our local flags, restore movement.

    // 1) Cancel light attack (combo state machine).
    CachedASC->StopAction(SharedGameplayTags::Action_LightAttack);
    bIsAttacking = false;
    bComboInputOpen = false;
    bComboQueued = false;
    ComboIndex = 0;

    // 2) Cancel heavy attack swing AND any in-progress charge.
    CachedASC->StopAction(SharedGameplayTags::Action_HeavyAttack);
    if (bIsHeavyCharging)
    {
        CancelHeavyCharge();
    }
    ResetHeavyChargeDamageState();

    // 4) Cancel block too — roll has full cancel priority.
    CachedASC->StopAction(SharedGameplayTags::Action_Block);

    // 5) Start the roll.
    CachedASC->StartAction(SharedGameplayTags::Action_Roll);

}

FVector ASoulsPlayerCharacter::GetRollDirectionWS() const
{
    // Convert cached 2D input (X=forward/back, Y=right/left) into world-space direction
    // using the same camera-relative basis as Move(). This way "S+Space" rolls behind
    // the camera, "A+Space" rolls left of camera, etc.
    if (LastMoveInput.IsNearlyZero())
    {
        return -GetActorForwardVector();   // standing still: roll backward
    }

    FRotator ControlRot = GetControlRotation();
    ControlRot.Pitch = 0.f;
    ControlRot.Roll = 0.f;

    const FVector ForwardWS = ControlRot.Vector();
    const FVector RightWS = FRotationMatrix(ControlRot).GetUnitAxis(EAxis::Y);

    return (ForwardWS * LastMoveInput.X + RightWS * LastMoveInput.Y).GetSafeNormal();
}


void ASoulsPlayerCharacter::OnBlockPressed()
{
    UE_LOG(LogGame, Log, TEXT("[Block] Pressed event fired"));

    if (!CachedASC || CachedASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead))
    {
        return;
    }

    // Symmetric to OnRollPressed: cancel anything in flight before raising guard.
    // Block has lower cancel priority than roll, but we still cancel ongoing
    // attacks/charges so the visual transition is clean.
    CachedASC->StopAction(SharedGameplayTags::Action_LightAttack);
    bIsAttacking = false;
    bComboInputOpen = false;
    bComboQueued = false;
    ComboIndex = 0;

    CachedASC->StopAction(SharedGameplayTags::Action_HeavyAttack);
    if (bIsHeavyCharging)
    {
        CancelHeavyCharge();
    }
    ResetHeavyChargeDamageState();

    // CanStart enforces BlockedTags (Status.IsStunned) and not-already-running.
    CachedASC->StartAction(SharedGameplayTags::Action_Block);
}

void ASoulsPlayerCharacter::OnBlockReleased()
{
    UE_LOG(LogGame, Log, TEXT("[Block] Released event fired @ %.2f"), GetWorld()->TimeSeconds);

    if (!CachedASC) return;

    // No-op if not currently blocking (e.g. guard-break already stopped it).
    CachedASC->StopAction(SharedGameplayTags::Action_Block);
}
