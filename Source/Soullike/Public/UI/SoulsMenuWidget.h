#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SoulsMenuWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class SOULLIKE_API USoulsMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void RebindMenuButtons();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    virtual void HandleStartClicked();

    UFUNCTION()
    virtual void HandleResumeClicked();

    UFUNCTION()
    virtual void HandleRestartClicked();

    UFUNCTION()
    virtual void HandleMainMenuClicked();

    UFUNCTION()
    virtual void HandleQuitClicked();

    void BindButton(UButton* Button, FName DebugName);
    void UnbindButton(UButton* Button, FName DebugName);

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UButton> StartButton = nullptr;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UButton> ResumeButton = nullptr;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UButton> RestartButton = nullptr;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UButton> MainMenuButton = nullptr;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UButton> QuitButton = nullptr;
};

UCLASS()
class SOULLIKE_API USoulsMainMenuWidget : public USoulsMenuWidget
{
    GENERATED_BODY()
};

UCLASS()
class SOULLIKE_API USoulsPauseMenuWidget : public USoulsMenuWidget
{
    GENERATED_BODY()
};

UCLASS()
class SOULLIKE_API USoulsResultWidget : public USoulsMenuWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void SetResultText(const FText& InTitle, const FText& InSubtitle);

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Souls|UI")
    FText DefaultTitle;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Souls|UI")
    FText DefaultSubtitle;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UTextBlock> TitleText = nullptr;

    UPROPERTY(meta = (BindWidget, OptionalWidget = true))
    TObjectPtr<UTextBlock> SubtitleText = nullptr;
};

UCLASS()
class SOULLIKE_API USoulsDeathWidget : public USoulsResultWidget
{
    GENERATED_BODY()

public:
    USoulsDeathWidget();
};

UCLASS()
class SOULLIKE_API USoulsVictoryWidget : public USoulsResultWidget
{
    GENERATED_BODY()

public:
    USoulsVictoryWidget();
};
