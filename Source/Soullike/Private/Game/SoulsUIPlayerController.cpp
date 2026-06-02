#include "Game/SoulsUIPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerInput.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ASoulsUIPlayerController::ASoulsUIPlayerController()
{
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
}

void ASoulsUIPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (bShowMainMenuOnBeginPlay)
    {
        ShowMainMenu();
    }
    else
    {
        ApplyGameInputMode();
    }
}

void ASoulsUIPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::TogglePauseMenu);
        InputComponent->BindKey(EKeys::P, IE_Pressed, this, &ThisClass::TogglePauseMenu);
    }
}

void ASoulsUIPlayerController::ShowMainMenu()
{
    ShowWidget(MainMenuWidgetClass, ESoulsUIScreen::MainMenu, 100, false);
}

void ASoulsUIPlayerController::ShowPauseMenu()
{
    if (!bCanTogglePause || ActiveScreen == ESoulsUIScreen::Death || ActiveScreen == ESoulsUIScreen::Victory)
    {
        return;
    }

    ShowWidget(PauseMenuWidgetClass, ESoulsUIScreen::Pause, 100, true);
}

void ASoulsUIPlayerController::ShowDeathScreen()
{
    ShowWidget(DeathWidgetClass, ESoulsUIScreen::Death, 200, false);
    SetIgnoreMoveInput(true);
    SetIgnoreLookInput(true);
}

void ASoulsUIPlayerController::ShowVictoryScreen()
{
    ShowWidget(VictoryWidgetClass, ESoulsUIScreen::Victory, 200, false);
}

void ASoulsUIPlayerController::HideMenuScreen()
{
    if (ActiveMenuWidget)
    {
        ActiveMenuWidget->RemoveFromParent();
        ActiveMenuWidget = nullptr;
    }

    ActiveScreen = ESoulsUIScreen::None;
}

void ASoulsUIPlayerController::StartGameFromMenu()
{
    const FName TargetLevel = ResolveGameplayLevelName();
    if (!TargetLevel.IsNone())
    {
        UGameplayStatics::SetGamePaused(this, false);
        UGameplayStatics::OpenLevel(this, TargetLevel);
    }
}

void ASoulsUIPlayerController::ResumeGame()
{
    UGameplayStatics::SetGamePaused(this, false);
    HideMenuScreen();
    ApplyGameInputMode();
}

void ASoulsUIPlayerController::RestartCurrentLevel()
{
    UGameplayStatics::SetGamePaused(this, false);
    const FName CurrentLevel = FName(*UGameplayStatics::GetCurrentLevelName(this, true));
    if (!CurrentLevel.IsNone())
    {
        UGameplayStatics::OpenLevel(this, CurrentLevel);
    }
}

void ASoulsUIPlayerController::ReturnToMainMenu()
{
    UGameplayStatics::SetGamePaused(this, false);

    const FName MenuLevel = ResolveMainMenuLevelName();
    if (!MenuLevel.IsNone())
    {
        UGameplayStatics::OpenLevel(this, MenuLevel);
        return;
    }

    ShowMainMenu();
}

void ASoulsUIPlayerController::QuitGame()
{
    UKismetSystemLibrary::QuitGame(this, this, EQuitPreference::Quit, true);
}

void ASoulsUIPlayerController::TogglePauseMenu()
{
    if (!bCanTogglePause || ActiveScreen == ESoulsUIScreen::Death || ActiveScreen == ESoulsUIScreen::Victory)
    {
        return;
    }

    if (ActiveScreen == ESoulsUIScreen::Pause)
    {
        ResumeGame();
    }
    else
    {
        ShowPauseMenu();
    }
}

UUserWidget* ASoulsUIPlayerController::ShowWidget(
    TSubclassOf<UUserWidget> WidgetClass, ESoulsUIScreen Screen, int32 ZOrder, bool bPauseGame)
{
    HideMenuScreen();

    if (!WidgetClass)
    {
        ActiveScreen = Screen;
        return nullptr;
    }

    ActiveMenuWidget = CreateWidget<UUserWidget>(this, WidgetClass);
    if (ActiveMenuWidget)
    {
        ActiveMenuWidget->AddToViewport(ZOrder);
    }

    ActiveScreen = Screen;
    if (bPauseGame)
    {
        UGameplayStatics::SetGamePaused(this, true);
    }

    ApplyUIInputMode();
    return ActiveMenuWidget;
}

void ASoulsUIPlayerController::ApplyGameInputMode()
{
    ResetIgnoreMoveInput();
    ResetIgnoreLookInput();
    bShowMouseCursor = false;
    bEnableClickEvents = false;
    bEnableMouseOverEvents = false;
    SetInputMode(FInputModeGameOnly());
}

void ASoulsUIPlayerController::ApplyUIInputMode()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    SetInputMode(FInputModeUIOnly());
}

FName ASoulsUIPlayerController::ResolveGameplayLevelName() const
{
    if (!GameplayLevelName.IsNone())
    {
        return GameplayLevelName;
    }

    return FName(*UGameplayStatics::GetCurrentLevelName(this, true));
}

FName ASoulsUIPlayerController::ResolveMainMenuLevelName() const
{
    return MainMenuLevelName;
}
