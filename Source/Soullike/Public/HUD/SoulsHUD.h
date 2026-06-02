#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SoulsHUD.generated.h"

class USoulsOverlay;
class USoulsActionSystemComponent;

UCLASS()
class SOULLIKE_API ASoulsHUD : public AHUD
{
    GENERATED_BODY()

public:
    ASoulsHUD();

    void SetHeavyChargeVisible(bool bVisible) const;
    void SetHeavyChargePercent(float Percent) const;
    void SetHeavyChargeStage(int32 StageIndex, int32 StageCount) const;

    UFUNCTION(BlueprintCallable, Category = "Souls|HUD")
    void ShowDeathScreen() const;

    UFUNCTION(BlueprintCallable, Category = "Souls|HUD")
    void ShowVictoryScreen() const;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|HUD")
    TSubclassOf<USoulsOverlay> OverlayWidgetClass;

    UPROPERTY(Transient)
    TObjectPtr<USoulsOverlay> OverlayWidget;
};
