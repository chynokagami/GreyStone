#include "ActionSystem/SoulsActionSystemComponent.h"

#include "Soullike.h"   // for LogGame
#include "SharedGameplayTags.h"
#include "ActionSystem/SoulsAction.h"
#include "ActionSystem/SoulsAttributeSet.h"


USoulsActionSystemComponent::USoulsActionSystemComponent()
{
    bWantsInitializeComponent = true;

    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void USoulsActionSystemComponent::InitializeComponent()
{
    Super::InitializeComponent();

    if (Attributes == nullptr)
    {
        Attributes = NewObject<USoulsAttributeSet>(this, USoulsAttributeSet::StaticClass());
        UE_LOG(LogGame, Warning, TEXT("No default AttributeSet defined. Set using SetDefaultAttributeSet() "
            "during Actor Construction or assign in Blueprint ActionComponent for %s."),
            *GetNameSafe(GetOwner()));
    }

    for (TFieldIterator<FStructProperty> PropIt(Attributes->GetClass()); PropIt; ++PropIt)
    {
        FSoulsAttribute* FoundAttribute = PropIt->ContainerPtrToValuePtr<FSoulsAttribute>(Attributes);

        FName AttributeTagName = FName(*FString("Attribute.") + PropIt->GetName());
        FGameplayTag AttributeTag = FGameplayTag::RequestGameplayTag(AttributeTagName);

        CachedAttributes.Add(AttributeTag, FoundAttribute);
    }

    for (TSubclassOf<USoulsAction> ActionClass : DefaultActions)
    {
        if (ensure(ActionClass))
        {
            GrantAction(ActionClass);
        }
    }
}

void USoulsActionSystemComponent::SetDefaultAttributeSet(TSubclassOf<USoulsAttributeSet> AttributeSetClass)
{
    check(!HasBeenInitialized());

    FObjectInitializer& ObjectInitializer = FObjectInitializer::Get();
    Attributes = Cast<USoulsAttributeSet>(
        ObjectInitializer.CreateDefaultSubobject(this, TEXT("Attributes"), AttributeSetClass, AttributeSetClass));
}

void USoulsActionSystemComponent::BeginPlay()
{
    Super::BeginPlay();

    Attributes->InitializeAttributes();
}

void USoulsActionSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    
    FSoulsAttribute* StaminaAttr = CachedAttributes.FindRef(SharedGameplayTags::Attribute_Stamina);
    FSoulsAttribute* StaminaMaxAttr = CachedAttributes.FindRef(SharedGameplayTags::Attribute_StaminaMax);
    if (!StaminaAttr || !StaminaMaxAttr)
    {
        return; 
    }

    
    static const FGameplayTag RegenRateTag = FGameplayTag::RequestGameplayTag(FName("Attribute.StaminaRegenRate"), false);
    static const FGameplayTag RegenDelayTag = FGameplayTag::RequestGameplayTag(FName("Attribute.StaminaRegenDelay"), false);

    const float RegenRate = RegenRateTag.IsValid() ? GetAttributeValue(RegenRateTag) : 45.f;
    const float RegenDelay = RegenDelayTag.IsValid() ? GetAttributeValue(RegenDelayTag) : 1.f;

    const float CurrentTime = GetWorld()->TimeSeconds;
    const float CurrentStamina = StaminaAttr->GetValue();
    const float MaxStamina = StaminaMaxAttr->GetValue();

    const bool bCanRegen = (CurrentTime - LastStaminaConsumeTime) >= RegenDelay
        && CurrentStamina < MaxStamina
        && !ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsBlocking)
        && !ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsExhausted);

    if (!bCanRegen) return;

    const float OldValue = StaminaAttr->GetValue();
    const float RegenAmount = RegenRate * DeltaTime;
    StaminaAttr->Modifier += RegenAmount;

    if (StaminaAttr->GetValue() > MaxStamina)
    {
        StaminaAttr->Modifier = MaxStamina - StaminaAttr->Base;
    }

    const float NewValue = StaminaAttr->GetValue();
    if (!FMath::IsNearlyEqual(NewValue, OldValue))
    {
        BroadcastAttributeChanged(SharedGameplayTags::Attribute_Stamina, NewValue, OldValue);
    }
}

void USoulsActionSystemComponent::GrantAction(TSubclassOf<USoulsAction> NewActionClass)
{
    if (!ensure(NewActionClass))
    {
        return;
    }
    USoulsAction* NewAction = NewObject<USoulsAction>(this, NewActionClass);
    Actions.Add(NewAction);
}

bool USoulsActionSystemComponent::StartAction(FGameplayTag InActionName)
{
    for (USoulsAction* Action : Actions)
    {
        if (Action->GetActionName() == InActionName)
        {
            if (Action->CanStart())
            {
                Action->StartAction();
                return Action->IsRunning();
            }
            return false;
        }
    }

    UE_LOG(LogGame, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
    return false;
}

void USoulsActionSystemComponent::StopAction(FGameplayTag InActionName)
{
    for (USoulsAction* Action : Actions)
    {
        if (Action->GetActionName() == InActionName)
        {
            if (Action->IsRunning())
            {
                Action->StopAction();
            }
            return;
        }
    }

    UE_LOG(LogGame, Warning, TEXT("No Action found with name %s"), *InActionName.ToString());
}

void USoulsActionSystemComponent::ApplyAttributeChange(FGameplayTag AttributeTag, float Delta, EAttributeModifyType ModifyType)
{
    FSoulsAttribute* FoundAttribute = GetAttribute(AttributeTag);
    if (!FoundAttribute)
    {
        UE_LOG(LogGame, Warning, TEXT("ApplyAttributeChange: attribute %s not found"), *AttributeTag.ToString());
        return;
    }

    const float OldValue = FoundAttribute->GetValue();

    switch (ModifyType)
    {
    case EAttributeModifyType::Base:
        FoundAttribute->Base += Delta;
        break;
    case EAttributeModifyType::Modifier:
        FoundAttribute->Modifier += Delta;
        break;
    case EAttributeModifyType::OverrideBase:
        FoundAttribute->Base = Delta;
        break;
    default:
        check(false);
    }

    Attributes->PostAttributeChanged();

    if (AttributeTag == SharedGameplayTags::Attribute_Stamina && Delta < 0.f)
    {
        LastStaminaConsumeTime = GetWorld()->TimeSeconds;
    }

    BroadcastAttributeChanged(AttributeTag, FoundAttribute->GetValue(), OldValue);

    UE_LOGFMT(LogGame, Log, "Attribute: {0}, New: {1}, Old: {2}",
        AttributeTag.ToString(),
        FoundAttribute->GetValue(),
        OldValue);
}

FSoulsAttribute* USoulsActionSystemComponent::GetAttribute(FGameplayTag InAttributeTag) const
{
    FSoulsAttribute* const* FoundAttribute = CachedAttributes.Find(InAttributeTag);
    return FoundAttribute ? *FoundAttribute : nullptr;
}

float USoulsActionSystemComponent::GetAttributeValue(FGameplayTag InAttributeTag) const
{
    FSoulsAttribute* FoundAttribute = GetAttribute(InAttributeTag);
    return FoundAttribute ? FoundAttribute->GetValue() : 0.f;
}

FOnAttributeChanged& USoulsActionSystemComponent::GetAttributeListener(FGameplayTag AttributeTag)
{
    return AttributeListeners.FindOrAdd(AttributeTag);
}

void USoulsActionSystemComponent::AddDynamicAttributeListener(FOnAttributeDynamicChanged Event, FGameplayTag AttributeTag)
{
    TArray<FOnAttributeDynamicChanged>& Events = AttributeDynamicListeners.FindOrAdd(AttributeTag);
    Events.Add(Event);
}

void USoulsActionSystemComponent::RemoveDynamicAttributeListener(FOnAttributeDynamicChanged Event)
{
    for (TPair<FGameplayTag, TArray<FOnAttributeDynamicChanged>>& Listener : AttributeDynamicListeners)
    {
        if (Listener.Value.RemoveSingle(Event) > 0)
        {
            UE_LOG(LogGame, Warning, TEXT("successfully removed blueprint binding."));
            break;
        }
    }
}

void USoulsActionSystemComponent::BroadcastAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue)
{
    if (FOnAttributeChanged* Event = AttributeListeners.Find(AttributeTag))
    {
        Event->Broadcast(AttributeTag, NewValue, OldValue);
    }

    if (TArray<FOnAttributeDynamicChanged>* Events = AttributeDynamicListeners.Find(AttributeTag))
    {
        for (int i = Events->Num() - 1; i >= 0; --i)
        {
            FOnAttributeDynamicChanged& Event = (*Events)[i];
            const bool bIsBound = Event.ExecuteIfBound(AttributeTag, NewValue, OldValue);
            if (!bIsBound)
            {
                Events->RemoveAt(i);
                UE_LOG(LogGame, Log, TEXT("Cleaned up expired attribute delegate for %s"), *GetNameSafe(GetOwner()));
            }
        }
    }
}
