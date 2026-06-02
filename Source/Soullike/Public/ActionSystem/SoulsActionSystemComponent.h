#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "SoulsActionSystemComponent.generated.h"


struct FSoulsAttribute;
class USoulsAttributeSet;
class USoulsAction;


UENUM(BlueprintType)
enum class EAttributeModifyType : uint8
{
    Base,           
    Modifier,       
    OverrideBase,   
    Invalid         UMETA(Hidden)
};


DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAttributeChanged, FGameplayTag /*AttributeTag*/, float /*NewAttributeValue*/, float /*OldAttributeValue*/);

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnAttributeDynamicChanged, FGameplayTag, AttributeTag, float, NewAttributeValue, float, OldAttributeValue);


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), HideCategories=(Navigation,Cooking,Tags))
class SOULLIKE_API USoulsActionSystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:

    USoulsActionSystemComponent();

    // Action
    bool StartAction(FGameplayTag InActionName);
    void StopAction(FGameplayTag InActionName);

    // AttributeSet 
    void SetDefaultAttributeSet(TSubclassOf<USoulsAttributeSet> AttributeSetClass);

    void GrantAction(TSubclassOf<USoulsAction> NewActionClass);

    UFUNCTION(BlueprintCallable)
    void ApplyAttributeChange(FGameplayTag AttributeTag, float Delta, EAttributeModifyType ModifyType);



    FSoulsAttribute* GetAttribute(FGameplayTag InAttributeTag) const;

    UFUNCTION(BlueprintCallable)
    float GetAttributeValue(FGameplayTag InAttributeTag) const;

    FOnAttributeChanged& GetAttributeListener(FGameplayTag AttributeTag);

    UFUNCTION(BlueprintCallable, DisplayName="Add Attribute Listener", meta=(Keywords="events,delegate"))
    void AddDynamicAttributeListener(FOnAttributeDynamicChanged Event, FGameplayTag AttributeTag);

    UFUNCTION(BlueprintCallable, DisplayName="Remove Attribute Listener", meta=(Keywords="events,delegate"))
    void RemoveDynamicAttributeListener(FOnAttributeDynamicChanged Event);

    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    FGameplayTagContainer ActiveGameplayTags;

protected:

    void BroadcastAttributeChanged(FGameplayTag AttributeTag, float NewValue, float OldValue);

    UPROPERTY(EditAnywhere, Instanced, NoClear, Category=ActionSystem)
    TObjectPtr<USoulsAttributeSet> Attributes;

    TMap<FGameplayTag, FSoulsAttribute*> CachedAttributes;

    TMap<FGameplayTag, FOnAttributeChanged> AttributeListeners;

    TMap<FGameplayTag, TArray<FOnAttributeDynamicChanged>> AttributeDynamicListeners;

    UPROPERTY()
    TArray<TObjectPtr<USoulsAction>> Actions;

    UPROPERTY(EditAnywhere, Category=ActionSystem)
    TArray<TSubclassOf<USoulsAction>> DefaultActions;

    float LastStaminaConsumeTime = 0.f;
};
