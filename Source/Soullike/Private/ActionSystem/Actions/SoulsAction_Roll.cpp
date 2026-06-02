#include "ActionSystem/Actions/SoulsAction_Roll.h"

#include "Soullike.h"
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Characters/SoulsPlayerCharacter.h"
#include "SharedGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/BlackboardKeys.h"

USoulsAction_Roll::USoulsAction_Roll()
{
    CooldownTime = 0.0f;
}

void USoulsAction_Roll::StartAction_Implementation()
{
    Super::StartAction_Implementation();

    USoulsActionSystemComponent* ASC = GetOwningComponent();
    if (!ASC) return;

    ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
    if (!Character)
    {
        UE_LOG(LogGame, Warning, TEXT("Roll: owner is not a Character"));
        StopAction();
        return;
    }

    if (!RollMontage)
    {
        UE_LOG(LogGame, Warning, TEXT("Roll: no RollMontage configured"));
        StopAction();
        return;
    }

    // Compute roll direction. Player caches its most recent WASD input; AI / debug
    // callers fall through to actor-back (default of GetRollDirectionWS).
    FVector RollDirWS = -Character->GetActorForwardVector();  // default: backward
    if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(Character))
    {
        RollDirWS = Player->GetRollDirectionWS();
    }

    // Snap actor rotation to face the roll direction. The montage's root motion
    // then carries the character forward in this new facing.
    if (!RollDirWS.IsNearlyZero())
    {
        const FRotator NewRot(0.f, RollDirWS.Rotation().Yaw, 0.f);
        Character->SetActorRotation(NewRot);
    }

    if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
    {
        Move->StopMovementImmediately();
    }

    const float MontageDuration = Character->PlayAnimMontage(RollMontage);

    UE_LOGFMT(LogGame, Log, "Roll: playing {Montage} for {Duration}s, dir=({DX},{DY})",
        ("Montage", RollMontage->GetName()),
        ("Duration", MontageDuration),
        ("DX", RollDirWS.X),
        ("DY", RollDirWS.Y));

    if (MontageDuration > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            RollTimerHandle, this, &ThisClass::OnRollTimerElapsed,
            MontageDuration, false);
    }
    else
    {
        UE_LOG(LogGame, Warning, TEXT("Roll: PlayAnimMontage returned 0 duration"));
        StopAction();
    }
}

void USoulsAction_Roll::StopAction_Implementation()
{
    Super::StopAction_Implementation();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RollTimerHandle);
    }

    // Defensive: if the IFrame notify state never closed (e.g. montage interrupted
    // by death/teleport), strip the tag so the character isn't permanently invincible.
    if (USoulsActionSystemComponent* ASC = GetOwningComponent())
    {
        ASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IFrame);

        ACharacter* Character = Cast<ACharacter>(ASC->GetOwner());
        if (!Character || Cast<ASoulsPlayerCharacter>(Character))
        {
            return;
        }

        AAIController* AICon = Character->GetController<AAIController>();
        if (!AICon)
        {
            return;  // AI not yet possessed (spawn race) or already unpossessed
        }

        UBlackboardComponent* BB = AICon->GetBlackboardComponent();
        if (!BB)
        {
            return;
        }

        AActor* Target = Cast<AActor>(BB->GetValueAsObject(SoulsBBKeys::TargetActor));
        if (!Target)
        {
            return;  // sight already lost / never acquired — nothing to face
        }

        const FVector ToTarget =
            (Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal2D();
        if (ToTarget.IsNearlyZero())
        {
            return;  // overlapping target — degenerate, skip
        }

        Character->SetActorRotation(ToTarget.Rotation());

        UE_LOG(LogGame, Verbose,
            TEXT("[Roll] %s post-roll snap to face %s"),
            *Character->GetName(), *Target->GetName());
    }
}

void USoulsAction_Roll::OnRollTimerElapsed()
{
    StopAction();
}