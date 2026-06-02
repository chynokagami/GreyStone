#include "ActionSystem/SoulsAction.h"

#include "Soullike.h"   // for LogGame
#include "ActionSystem/SoulsActionSystemComponent.h"


void USoulsAction::StartAction_Implementation()
{
    bIsRunning = true;

    const float GameTime = GetWorld()->TimeSeconds;

    UE_LOGFMT(LogGame, Log, "Started Action {ActionName} - {WorldTime}",
        ("ActionName", ActionName.ToString()),
        ("WorldTime", GameTime));

    // add GrantTags
    GetOwningComponent()->ActiveGameplayTags.AppendTags(GrantTags);

    // every ActivationCost ApplyAttributeChange
    // UI will info
    for (const TPair<FGameplayTag, float>& Cost : ActivationCost)
    {
        GetOwningComponent()->ApplyAttributeChange(Cost.Key, -Cost.Value, EAttributeModifyType::Modifier);
    }
}

void USoulsAction::StopAction_Implementation()
{
    bIsRunning = false;

    const float GameTime = GetWorld()->TimeSeconds;

    UE_LOG(LogGame, Log, TEXT("Stopped Action %s @ %f"), *ActionName.ToString(), GameTime);


    CooldownUntil = GameTime + CooldownTime;

    // remove GrantTags
    GetOwningComponent()->ActiveGameplayTags.RemoveTags(GrantTags);
}

bool USoulsAction::CanStart() const
{
    if (IsRunning())
    {
        return false;
    }
    if (CooldownTime > 0.0f && GetCooldownTimeRemaining() > 0.0f)
    {
        UE_LOG(LogGame, Log, TEXT("Cooldown remaining: %f"), GetCooldownTimeRemaining());
        UE_LOG(LogGame, Verbose, TEXT("Action %s on cooldown: %.2fs"),
            *ActionName.ToString(), GetCooldownTimeRemaining());

        return false;
    }

    USoulsActionSystemComponent* OwningComp = GetOwningComponent();
    if (!OwningComp) return false;

    if (OwningComp->ActiveGameplayTags.HasAny(BlockedTags))
    {
        UE_LOG(LogGame, Verbose, TEXT("Action %s blocked by ActiveTags"), *ActionName.ToString())
            return false;
    }

    for (const TPair<FGameplayTag, float>& Cost : ActivationCost)
    {
        const float AvailableAttributeAmount = OwningComp->GetAttributeValue(Cost.Key);
        if (AvailableAttributeAmount < Cost.Value)
        {
            UE_LOGFMT(LogGame, Log, "Not enough {AttributeName} to activate {ActionName}. "
                "Have {AvailableAttributeValue} and need {RequiredAttributeValue}",
                ("AttributeName", Cost.Key.ToString()),
                ("ActionName", ActionName.ToString()),
                ("AvailableAttributeValue", AvailableAttributeAmount),
                ("RequiredAttributeValue", Cost.Value));
            return false;
        }
    }

    return true;
}

float USoulsAction::GetCooldownTimeRemaining() const
{
    return FMath::Max(0.0f, CooldownUntil - GetWorld()->TimeSeconds);
}

USoulsActionSystemComponent* USoulsAction::GetOwningComponent() const
{
    // GetOuter() is Outer—— ActionSystemComponent
    return Cast<USoulsActionSystemComponent>(GetOuter());
}