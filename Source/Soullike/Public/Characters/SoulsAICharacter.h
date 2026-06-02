#pragma once

#include "CoreMinimal.h"
#include "Characters/SoulsBaseCharacter.h"
#include "SoulsAICharacter.generated.h"

UCLASS(Abstract)
class SOULLIKE_API ASoulsAICharacter : public ASoulsBaseCharacter
{
    GENERATED_BODY()

public:
    ASoulsAICharacter();

protected:
    //~ ASoulsBaseCharacter
    virtual void Die() override;
    //~ End ASoulsBaseCharacter

    UPROPERTY(EditDefaultsOnly, Category = "Souls|AI", meta = (ClampMin = "0.5", UIMin = "0.5"))
    float CorpseLifetime = 8.0f;

};