#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoulsUIPlayerController.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class ESoulsUIScreen : uint8
{
    None,
    MainMenu,
    Pause,
    Death,
    Victory
};

UCLASS()
class SOULLIKE_API ASoulsUIPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ASoulsUIPlayerController();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ShowMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ShowPauseMenu();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ShowDeathScreen();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ShowVictoryScreen();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void HideMenuScreen();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void StartGameFromMenu();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ResumeGame();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void RestartCurrentLevel();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void ReturnToMainMenu();

    UFUNCTION(BlueprintCallable, Category = "Souls|UI")
    void QuitGame();

    UFUNCTION(BlueprintPure, Category = "Souls|UI")
    ESoulsUIScreen GetActiveUIScreen() const { return ActiveScreen; }

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;

    UFUNCTION()
    void TogglePauseMenu();

    UUserWidget* ShowWidget(TSubclassOf<UUserWidget> WidgetClass, ESoulsUIScreen Screen, int32 ZOrder, bool bPauseGame);
    void ApplyGameInputMode();
    void ApplyUIInputMode();
    FName ResolveGameplayLevelName() const;
    FName ResolveMainMenuLevelName() const;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Classes")
    TSubclassOf<UUserWidget> MainMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Classes")
    TSubclassOf<UUserWidget> PauseMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Classes")
    TSubclassOf<UUserWidget> DeathWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Classes")
    TSubclassOf<UUserWidget> VictoryWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Maps")
    FName GameplayLevelName = TEXT("Lvl_Game");

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI|Maps")
    FName MainMenuLevelName = NAME_None;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI")
    bool bShowMainMenuOnBeginPlay = false;

    UPROPERTY(EditDefaultsOnly, Category = "Souls|UI")
    bool bCanTogglePause = true;

    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveMenuWidget = nullptr;

    UPROPERTY(Transient)
    ESoulsUIScreen ActiveScreen = ESoulsUIScreen::None;
};
