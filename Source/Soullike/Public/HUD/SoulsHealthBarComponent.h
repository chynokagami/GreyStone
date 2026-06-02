#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Delegates/IDelegateInstance.h"   // FDelegateHandle
#include "GameplayTagContainer.h"   
#include "SoulsHealthBarComponent.generated.h"

class USoulsHealthBar;
class USoulsActionSystemComponent;
class ASoulsBaseCharacter;

UCLASS(ClassGroup = (Souls), meta = (BlueprintSpawnableComponent))
class SOULLIKE_API USoulsHealthBarComponent : public UWidgetComponent
{
    GENERATED_BODY()

public:
    USoulsHealthBarComponent();

protected:
    //~ UActorComponent
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, enum ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;
    //~ End UActorComponent

    // Internal driving methods

    /** Resolves the cached widget pointer (lazy on first use). */
    USoulsHealthBar* ResolveWidget();

    /** Pull current Health / HealthMax from ASC and push percentage to widget. */
    void RefreshFromASC();

    // Event callbacks
    void HandleHealthChanged(FGameplayTag AttributeTag, float NewValue, float OldValue);

    UFUNCTION()
    void HandleOwnerDeath(ASoulsBaseCharacter* DeadCharacter);

    // Cached references

    UPROPERTY(Transient)
    TObjectPtr<USoulsHealthBar> CachedWidget = nullptr;

    UPROPERTY(Transient)
    TWeakObjectPtr<USoulsActionSystemComponent> CachedASC = nullptr;

    FDelegateHandle HealthListenerHandle;
    FDelegateHandle HealthMaxListenerHandle;

    // Tunables
    /** Hide the bar at 100% HP. Standard souls behavior; set false for boss bars. */
    UPROPERTY(EditDefaultsOnly, Category = "Souls|HUD")
    bool bHideAtFullHealth = true;
};
