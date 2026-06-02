#include "ActionSystem/Actions/SoulsAction_HeavyAttack.h"

#include "Soullike.h"   // LogGame
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Characters/SoulsPlayerCharacter.h"

USoulsAction_HeavyAttack::USoulsAction_HeavyAttack()
{
    CooldownTime = 0.0f;
}

void USoulsAction_HeavyAttack::StartAction_Implementation()
{
    Super::StartAction_Implementation();

    USoulsActionSystemComponent* ASC = GetOwningComponent();
    if (!ASC) return;

    ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
    if (!Character)
    {
        UE_LOG(LogGame, Warning, TEXT("HeavyAttack: owner is not a Character"));
        StopAction();
        return;
    }

    // Pick montage based on charge level the player computed at release time.
    // Fall back for non-player callers (debug / AI / scripted).
    UAnimMontage* MontageToPlay = nullptr;
    if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(Character))
    {
        MontageToPlay = Player->GetCurrentHeavyMontage();
    }
    if (!MontageToPlay)
    {
        MontageToPlay = FallbackMontage;
    }

    if (!MontageToPlay)
    {
        UE_LOG(LogGame, Warning,
            TEXT("HeavyAttack: no montage to play (Light/Full both null AND FallbackMontage null)"));
        StopAction();
        return;
    }

    const float MontageDuration = Character->PlayAnimMontage(MontageToPlay);

    UE_LOGFMT(LogGame, Log, "HeavyAttack: playing {Montage} for {Duration}s",
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
        UE_LOG(LogGame, Warning, TEXT("HeavyAttack: PlayAnimMontage returned 0 duration"));
        StopAction();
    }
}

void USoulsAction_HeavyAttack::StopAction_Implementation()
{
    Super::StopAction_Implementation();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(AttackTimerHandle);
    }

    if (USoulsActionSystemComponent* ASC = GetOwningComponent())
    {
        if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(ASC->GetOwner()))
        {
            Player->OnHeavyAttackActionEnded();
        }
    }
}

void USoulsAction_HeavyAttack::OnAttackTimerElapsed()
{
    StopAction();
}
