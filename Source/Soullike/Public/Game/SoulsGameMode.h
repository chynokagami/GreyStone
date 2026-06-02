#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SoulsGameMode.generated.h"

UCLASS()
class SOULLIKE_API ASoulsGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ASoulsGameMode();
};

/** Dedicated main-menu mode: no pawn spawn, menu shown by the PlayerController. */
UCLASS()
class SOULLIKE_API ASoulsMainMenuGameMode : public ASoulsGameMode
{
    GENERATED_BODY()

public:
    ASoulsMainMenuGameMode();
};
