#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulsHealthBar.generated.h"

class UProgressBar;

UCLASS()
class SOULLIKE_API USoulsHealthBar : public UUserWidget
{
    GENERATED_BODY()

public:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> HealthBarProgress;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UProgressBar> PostureBarProgress;
};