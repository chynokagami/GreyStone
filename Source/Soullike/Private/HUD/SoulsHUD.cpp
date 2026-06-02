#include "HUD/SoulsHUD.h"

#include "Soullike.h"                                          // LogGame
#include "HUD/SoulsOverlay.h"
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Game/SoulsUIPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ASoulsHUD::ASoulsHUD()
{
    // AHUD doesn't tick by default and we don't need it to — overlay self-drives.
    // Leaving PrimaryActorTick.bCanEverTick = false (AHUD default).
}

void ASoulsHUD::SetHeavyChargeVisible(bool bVisible) const
{
    if (OverlayWidget)
    {
        OverlayWidget->SetHeavyChargeVisible(bVisible);
    }
}

void ASoulsHUD::SetHeavyChargePercent(float Percent) const
{
    if (OverlayWidget)
    {
        OverlayWidget->SetHeavyChargePercent(Percent);
    }
}

void ASoulsHUD::SetHeavyChargeStage(int32 StageIndex, int32 StageCount) const
{
    if (OverlayWidget)
    {
        OverlayWidget->SetHeavyChargeStage(StageIndex, StageCount);
    }
}

void ASoulsHUD::ShowDeathScreen() const
{
    if (ASoulsUIPlayerController* PC = Cast<ASoulsUIPlayerController>(GetOwningPlayerController()))
    {
        PC->ShowDeathScreen();
    }
}

void ASoulsHUD::ShowVictoryScreen() const
{
    if (ASoulsUIPlayerController* PC = Cast<ASoulsUIPlayerController>(GetOwningPlayerController()))
    {
        PC->ShowVictoryScreen();
    }
}

void ASoulsHUD::BeginPlay()
{
    Super::BeginPlay();

    // Sanity: must have a widget class configured
    if (!OverlayWidgetClass)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[SoulsHUD] %s has no OverlayWidgetClass set in BP CDO — no UI"),
            *GetName());
        return;
    }

    // esolve owning player + ASC
    APlayerController* PC = GetOwningPlayerController();
    if (!PC)
    {
        UE_LOG(LogGame, Warning, TEXT("[SoulsHUD] No owning PlayerController"));
        return;
    }

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn)
    {
        // Possible during pre-PossessedBy timing. Defer once via timer if it
        // becomes a real issue; for now, log and bail (player is mid-spawn).
        UE_LOG(LogGame, Warning,
            TEXT("[SoulsHUD] PC has no Pawn yet at HUD::BeginPlay — UI will not init"));
        return;
    }

    USoulsActionSystemComponent* ASC =
        PlayerPawn->FindComponentByClass<USoulsActionSystemComponent>();
    if (!ASC)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[SoulsHUD] Pawn %s has no ActionSystemComponent — UI will not bind"),
            *PlayerPawn->GetName());
        return;
    }

    // Create + inject + add to viewport
    OverlayWidget = CreateWidget<USoulsOverlay>(PC, OverlayWidgetClass);
    if (!OverlayWidget)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[SoulsHUD] CreateWidget returned null — check OverlayWidgetClass parent"));
        return;
    }

    OverlayWidget->InitializeOverlay(ASC);

    OverlayWidget->AddToViewport(0);

    UE_LOG(LogGame, Log,
        TEXT("[SoulsHUD] Overlay created and bound to ASC on %s"),
        *PlayerPawn->GetName());
}
