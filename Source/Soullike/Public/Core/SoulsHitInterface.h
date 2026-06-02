#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SoulsHitInterface.generated.h"

// MinimalAPI
// Blueprintable
UINTERFACE(MinimalAPI, Blueprintable)
class USoulsHitInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * Anything that can be hit by a damaging source (player, AI, breakable).
 * Implement on AActor subclasses to receive directional hit-react callbacks.
 */
class SOULLIKE_API ISoulsHitInterface
{
    GENERATED_BODY()

public:
    /**
     * Fired when this actor receives a hit.
     * @param ImpactPoint  World-space location of the hit. Used to compute hit direction.
     * @param Hitter       The actor that delivered the hit (often the weapon's owner). May be null.
     *
     * Default impl in C++ runs DirectionalHitReact + montage/sound/VFX.
     * BP can override _Implementation to add custom behavior.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Souls|Combat")
    void GetHit(const FVector& ImpactPoint, AActor* Hitter);
};