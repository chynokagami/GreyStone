#include "HUD/SoulsOverlay.h"

#include "Soullike.h"                                          // LogGame
#include "SharedGameplayTags.h"                                // SharedGameplayTags::Attribute_*
#include "ActionSystem/SoulsActionSystemComponent.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"                            // SetPercent
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"

void USoulsOverlay::InitializeOverlay(USoulsActionSystemComponent* InOwnerASC)
{
    OwnerASC = InOwnerASC;

    if (!InOwnerASC)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[SoulsOverlay] InitializeOverlay called with null ASC — bars will not update"));
    }
}

void USoulsOverlay::NativeConstruct()
{
    Super::NativeConstruct();

    if (!HeavyChargeBarProgress)
    {
        if (UVerticalBox* RootStack = Cast<UVerticalBox>(GetWidgetFromName(TEXT("VerticalBox_66"))))
        {
            HeavyChargeBarProgress = WidgetTree
                ? WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("RuntimeHeavyChargeBarProgress"))
                : nullptr;
            if (HeavyChargeBarProgress)
            {
                HeavyChargeBarProgress->SetPercent(0.f);
                HeavyChargeBarProgress->SetVisibility(ESlateVisibility::Collapsed);

                if (UVerticalBoxSlot* HeavyChargeSlot = RootStack->AddChildToVerticalBox(HeavyChargeBarProgress))
                {
                    HeavyChargeSlot->SetPadding(FMargin(0.f, 6.f, 0.f, 0.f));
                }
            }
        }
    }

    SetHeavyChargeVisible(false);

    // Subscribe + initial paint
    SubscribeAttributes();
    RefreshAllBars();
}

void USoulsOverlay::NativeDestruct()
{
    UnsubscribeAttributes();

    Super::NativeDestruct();
}

void USoulsOverlay::SubscribeAttributes()
{
    USoulsActionSystemComponent* ASC = OwnerASC.Get();
    if (!ASC)
    {
        return;
    }

    // Health
    // GetAttributeListener returns the per-attribute multicast by reference.
    // AddUObject returns a FDelegateHandle which we store for clean Remove().
    HealthListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
        .AddUObject(this, &USoulsOverlay::HandleHealthChanged);
    HealthMaxListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_HealthMax)
        .AddUObject(this, &USoulsOverlay::HandleHealthChanged);

    // Stamina
    // Different attribute = different internal multicast in ASC =
    // different handle. Cannot share with Health.
    StaminaListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_Stamina)
        .AddUObject(this, &USoulsOverlay::HandleStaminaChanged);
    StaminaMaxListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_StaminaMax)
        .AddUObject(this, &USoulsOverlay::HandleStaminaChanged);

}

void USoulsOverlay::UnsubscribeAttributes()
{
    USoulsActionSystemComponent* ASC = OwnerASC.Get();

    // Health
    if (HealthListenerHandle.IsValid())
    {
        if (ASC)
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
               .Remove(HealthListenerHandle);
        }
        HealthListenerHandle.Reset();
    }
    if (HealthMaxListenerHandle.IsValid())
    {
        if (ASC)
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_HealthMax)
               .Remove(HealthMaxListenerHandle);
        }
        HealthMaxListenerHandle.Reset();
    }

    // Stamina
    if (StaminaListenerHandle.IsValid())
    {
        if (ASC)
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_Stamina)
               .Remove(StaminaListenerHandle);
        }
        StaminaListenerHandle.Reset();
    }
    if (StaminaMaxListenerHandle.IsValid())
    {
        if (ASC)
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_StaminaMax)
               .Remove(StaminaMaxListenerHandle);
        }
        StaminaMaxListenerHandle.Reset();
    }
}

void USoulsOverlay::RefreshAllBars()
{
    USoulsActionSystemComponent* ASC = OwnerASC.Get();
    if (!ASC)
    {
        return;
    }

    // Health
    if (HealthBarProgress)
    {
        const float Health    = ASC->GetAttributeValue(SharedGameplayTags::Attribute_Health);
        const float HealthMax = ASC->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);

        const float Percent = (HealthMax > KINDA_SMALL_NUMBER)
                              ? FMath::Clamp(Health / HealthMax, 0.0f, 1.0f)
                              : 0.0f;
        HealthBarProgress->SetPercent(Percent);
    }

    // Stamina
    if (StaminaBarProgress)
    {
        const float Stamina    = ASC->GetAttributeValue(SharedGameplayTags::Attribute_Stamina);
        const float StaminaMax = ASC->GetAttributeValue(SharedGameplayTags::Attribute_StaminaMax);

        const float Percent = (StaminaMax > KINDA_SMALL_NUMBER)
                              ? FMath::Clamp(Stamina / StaminaMax, 0.0f, 1.0f)
                              : 0.0f;
        StaminaBarProgress->SetPercent(Percent);
    }

}

void USoulsOverlay::HandleHealthChanged(
    FGameplayTag /*AttributeTag*/, float /*NewValue*/, float /*OldValue*/)
{
    // Don't trust NewValue alone — HealthMax may have changed in the same
    // frame (buff tick, equipment swap). Re-read both via RefreshAllBars
    // for self-consistent ratio.
    RefreshAllBars();
}

void USoulsOverlay::HandleStaminaChanged(
    FGameplayTag /*AttributeTag*/, float /*NewValue*/, float /*OldValue*/)
{
    // Same reasoning as Health — re-read for ratio consistency.
    RefreshAllBars();
}

void USoulsOverlay::SetHeavyChargeVisible(bool bVisible)
{
    if (HeavyChargeBarProgress)
    {
        HeavyChargeBarProgress->SetVisibility(
            bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
    }
}

void USoulsOverlay::SetHeavyChargePercent(float Percent)
{
    if (HeavyChargeBarProgress)
    {
        HeavyChargeBarProgress->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
    }
}

void USoulsOverlay::SetHeavyChargeStage(int32 StageIndex, int32 StageCount)
{
    if (!HeavyChargeBarProgress)
    {
        return;
    }

    const float SafeStageCount = FMath::Max(1, StageCount);
    const float Alpha = FMath::Clamp(static_cast<float>(StageIndex) / static_cast<float>(SafeStageCount), 0.f, 1.f);
    const FLinearColor StageColor = FMath::Lerp(
        FLinearColor(0.28f, 0.62f, 1.0f, 1.0f),
        FLinearColor(1.0f, 0.45f, 0.12f, 1.0f),
        Alpha);

    HeavyChargeBarProgress->SetFillColorAndOpacity(StageColor);
}
