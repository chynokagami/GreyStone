#include "Game/SoulsGameMode.h"

#include "Game/SoulsUIPlayerController.h"
#include "HUD/SoulsHUD.h"

ASoulsGameMode::ASoulsGameMode()
{
    PlayerControllerClass = ASoulsUIPlayerController::StaticClass();
    HUDClass = ASoulsHUD::StaticClass();
}

ASoulsMainMenuGameMode::ASoulsMainMenuGameMode()
{
    PlayerControllerClass = ASoulsUIPlayerController::StaticClass();
    DefaultPawnClass = nullptr;
    HUDClass = nullptr;
}
