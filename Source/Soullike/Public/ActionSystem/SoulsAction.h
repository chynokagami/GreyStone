#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "SoulsAction.generated.h"


class USoulsActionSystemComponent;



UCLASS(Blueprintable, Abstract)
class SOULLIKE_API USoulsAction : public UObject
{
    GENERATED_BODY()

protected:

    UPROPERTY(EditDefaultsOnly, Category="Actions")
    FGameplayTag ActionName;

    UPROPERTY(EditDefaultsOnly, Category="Actions")
    FGameplayTagContainer GrantTags;

    UPROPERTY(EditDefaultsOnly, Category="Actions")
    FGameplayTagContainer BlockedTags;

    UPROPERTY(EditDefaultsOnly, Category="Actions")
    float CooldownTime = 0.0f;

    UPROPERTY(EditDefaultsOnly, Category="Actions")
    TMap<FGameplayTag, float> ActivationCost;

public:

    UFUNCTION(BlueprintCallable)
    USoulsActionSystemComponent* GetOwningComponent() const;

    bool CanStart() const;

    bool IsRunning() const
    {
        return bIsRunning;
    }

    UFUNCTION(BlueprintNativeEvent, Category="Actions")
    void StartAction();

    UFUNCTION(BlueprintNativeEvent, Category="Actions")
    void StopAction();

    float GetCooldownTimeRemaining() const;

    FGameplayTag GetActionName() const
    {
        return ActionName;
    }

protected:

    UPROPERTY(Transient)
    float CooldownUntil = 0.f;

    UPROPERTY(Transient)
    bool bIsRunning = false;
};