#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Delegates/IDelegateInstance.h"   // FDelegateHandle
#include "GameplayTagContainer.h"          // FGameplayTag (passed by value in callbacks)
#include "SoulsOverlay.generated.h"

class UProgressBar;
class USoulsActionSystemComponent;

UCLASS()
class SOULLIKE_API USoulsOverlay : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeOverlay(USoulsActionSystemComponent* InOwnerASC);

    UFUNCTION(BlueprintCallable, Category="Souls|HUD")
    void SetHeavyChargeVisible(bool bVisible);

    UFUNCTION(BlueprintCallable, Category="Souls|HUD")
    void SetHeavyChargePercent(float Percent);

    UFUNCTION(BlueprintCallable, Category="Souls|HUD")
    void SetHeavyChargeStage(int32 StageIndex, int32 StageCount);

protected:
    //~ UUserWidget
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    //~ End UUserWidget

    // Internal driving methods

    /** Subscribe to all attributes we care about. Called from NativeConstruct. */
    void SubscribeAttributes();

    /** Unsubscribe from all. Called from NativeDestruct. */
    void UnsubscribeAttributes();

    /** Push current Health %, Stamina % to the BindWidget progress bars. */
    void RefreshAllBars();

    // Event callbacks
    void HandleHealthChanged(FGameplayTag AttributeTag, float NewValue, float OldValue);
    void HandleStaminaChanged(FGameplayTag AttributeTag, float NewValue, float OldValue);

    // BindWidget refs
    // Filled by UMG BP at runtime via reflection on member name.
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBarProgress;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> StaminaBarProgress;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UProgressBar> PostureBarProgress;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UProgressBar> HeavyChargeBarProgress;

    // Cached references
    UPROPERTY(Transient)
    TWeakObjectPtr<USoulsActionSystemComponent> OwnerASC = nullptr;

    FDelegateHandle HealthListenerHandle;
    FDelegateHandle HealthMaxListenerHandle;
    FDelegateHandle StaminaListenerHandle;
    FDelegateHandle StaminaMaxListenerHandle;
};
