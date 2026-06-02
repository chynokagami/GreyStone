#include "ActionSystem/Actions/SoulsAction_LightAttack.h"

#include "Soullike.h"   // LogGame
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Characters/SoulsPlayerCharacter.h"

USoulsAction_LightAttack::USoulsAction_LightAttack()
{
    CooldownTime = 0.0f;
}

void USoulsAction_LightAttack::StartAction_Implementation()
{
    Super::StartAction_Implementation();

    USoulsActionSystemComponent* ASC = GetOwningComponent();
    if (!ASC) return;

    ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
    if (!Character)
    {
        UE_LOG(LogGame, Warning, TEXT("LightAttack: owner is not a Character"));
        StopAction();
        return;
    }

    // Pick the current combo segment from the player; fall back to AttackMontage for
    // non-combo callers (debug, AI, single-shot).
    UAnimMontage* MontageToPlay = nullptr;
    if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(Character))
    {
        MontageToPlay = Player->GetCurrentComboMontage();
    }
    if (!MontageToPlay)
    {
        MontageToPlay = AttackMontage;
    }

    if (!MontageToPlay)
    {
        UE_LOG(LogGame, Warning,
            TEXT("LightAttack: no montage to play (combo list empty AND fallback AttackMontage null)"));
        StopAction();
        return;
    }

    const float MontageDuration = Character->PlayAnimMontage(MontageToPlay);

        UE_LOGFMT(LogGame, Log, "LightAttack: playing {Montage} for {Duration}s",
        ("Montage", MontageToPlay->GetName()),
        ("Duration", MontageDuration));

    if (MontageDuration > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            AttackTimerHandle, this, &ThisClass::OnAttackTimerElapsed,
            MontageDuration, false);
    }
    else
    {
        UE_LOG(LogGame, Warning, TEXT("LightAttack: PlayAnimMontage returned 0 duration"));
        StopAction();
    }
}

void USoulsAction_LightAttack::StopAction_Implementation()
{
    Super::StopAction_Implementation();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AttackTimerHandle);
    }
}

void USoulsAction_LightAttack::OnAttackTimerElapsed()
{
    StopAction();

    if (USoulsActionSystemComponent* ASC = GetOwningComponent())
    {
        if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(ASC->GetOwner()))
        {
            if (Player->IsAttacking())
            {
                Player->TryAdvanceComboOrReset();
            }
        }
    }
}
