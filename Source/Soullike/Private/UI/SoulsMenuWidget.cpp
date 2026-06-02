#include "UI/SoulsMenuWidget.h"

#include "Soullike.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Game/SoulsUIPlayerController.h"

void USoulsMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    RebindMenuButtons();
}

void USoulsMenuWidget::NativeDestruct()
{
    UnbindButton(StartButton, TEXT("StartButton"));
    UnbindButton(ResumeButton, TEXT("ResumeButton"));
    UnbindButton(RestartButton, TEXT("RestartButton"));
    UnbindButton(MainMenuButton, TEXT("MainMenuButton"));
    UnbindButton(QuitButton, TEXT("QuitButton"));

    Super::NativeDestruct();
}

void USoulsMenuWidget::RebindMenuButtons()
{
    BindButton(StartButton, TEXT("StartButton"));
    BindButton(ResumeButton, TEXT("ResumeButton"));
    BindButton(RestartButton, TEXT("RestartButton"));
    BindButton(MainMenuButton, TEXT("MainMenuButton"));
    BindButton(QuitButton, TEXT("QuitButton"));
}

void USoulsMenuWidget::HandleStartClicked()
{
    if (ASoulsUIPlayerController* PC = GetOwningPlayer<ASoulsUIPlayerController>())
    {
        PC->StartGameFromMenu();
    }
}

void USoulsMenuWidget::HandleResumeClicked()
{
    if (ASoulsUIPlayerController* PC = GetOwningPlayer<ASoulsUIPlayerController>())
    {
        PC->ResumeGame();
    }
}

void USoulsMenuWidget::HandleRestartClicked()
{
    if (ASoulsUIPlayerController* PC = GetOwningPlayer<ASoulsUIPlayerController>())
    {
        PC->RestartCurrentLevel();
    }
}

void USoulsMenuWidget::HandleMainMenuClicked()
{
    if (ASoulsUIPlayerController* PC = GetOwningPlayer<ASoulsUIPlayerController>())
    {
        PC->ReturnToMainMenu();
    }
}

void USoulsMenuWidget::HandleQuitClicked()
{
    if (ASoulsUIPlayerController* PC = GetOwningPlayer<ASoulsUIPlayerController>())
    {
        PC->QuitGame();
    }
}

void USoulsMenuWidget::BindButton(UButton* Button, FName DebugName)
{
    if (!Button)
    {
        UE_LOG(LogGame, Verbose, TEXT("[UI] %s missing on %s"), *DebugName.ToString(), *GetName());
        return;
    }

    UnbindButton(Button, DebugName);

    if (DebugName == TEXT("StartButton"))
    {
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleStartClicked);
    }
    else if (DebugName == TEXT("ResumeButton"))
    {
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleResumeClicked);
    }
    else if (DebugName == TEXT("RestartButton"))
    {
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleRestartClicked);
    }
    else if (DebugName == TEXT("MainMenuButton"))
    {
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleMainMenuClicked);
    }
    else if (DebugName == TEXT("QuitButton"))
    {
        Button->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleQuitClicked);
    }
}

void USoulsMenuWidget::UnbindButton(UButton* Button, FName DebugName)
{
    if (!Button)
    {
        return;
    }

    if (DebugName == TEXT("StartButton"))
    {
        Button->OnClicked.RemoveDynamic(this, &ThisClass::HandleStartClicked);
    }
    else if (DebugName == TEXT("ResumeButton"))
    {
        Button->OnClicked.RemoveDynamic(this, &ThisClass::HandleResumeClicked);
    }
    else if (DebugName == TEXT("RestartButton"))
    {
        Button->OnClicked.RemoveDynamic(this, &ThisClass::HandleRestartClicked);
    }
    else if (DebugName == TEXT("MainMenuButton"))
    {
        Button->OnClicked.RemoveDynamic(this, &ThisClass::HandleMainMenuClicked);
    }
    else if (DebugName == TEXT("QuitButton"))
    {
        Button->OnClicked.RemoveDynamic(this, &ThisClass::HandleQuitClicked);
    }
}

void USoulsResultWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetResultText(DefaultTitle, DefaultSubtitle);
}

void USoulsResultWidget::SetResultText(const FText& InTitle, const FText& InSubtitle)
{
    if (TitleText)
    {
        TitleText->SetText(InTitle);
    }

    if (SubtitleText)
    {
        SubtitleText->SetText(InSubtitle);
    }
}

USoulsDeathWidget::USoulsDeathWidget()
{
    DefaultTitle = NSLOCTEXT("SoulsUI", "DeathTitle", "YOU DIED");
    DefaultSubtitle = NSLOCTEXT("SoulsUI", "DeathSubtitle", "Try again");
}

USoulsVictoryWidget::USoulsVictoryWidget()
{
    DefaultTitle = NSLOCTEXT("SoulsUI", "VictoryTitle", "VICTORY");
    DefaultSubtitle = NSLOCTEXT("SoulsUI", "VictorySubtitle", "Enemy defeated");
}
