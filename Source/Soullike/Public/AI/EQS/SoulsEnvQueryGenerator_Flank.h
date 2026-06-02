#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "EnvironmentQuery/EnvQueryTypes.h"           // FEnvQueryInstance (in virtual override sig)
#include "DataProviders/AIDataProvider.h"             // FAIDataProviderFloatValue / IntValue
#include "SoulsEnvQueryGenerator_Flank.generated.h"

UCLASS(meta = (DisplayName = "Souls Flank Point"))
class SOULLIKE_API USoulsEnvQueryGenerator_Flank : public UEnvQueryGenerator_ProjectedPoints
{
    GENERATED_BODY()

public:
    USoulsEnvQueryGenerator_Flank();

protected:
    //~ UEnvQueryGenerator interface
    virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;

    /** Description shown in EQS editor under the node title. */
    virtual FText GetDescriptionTitle() const override;
    virtual FText GetDescriptionDetails() const override;
    //~ End UEnvQueryGenerator

    // Configurable parameters
    UPROPERTY(EditAnywhere, Category = "Generator")
    TSubclassOf<UEnvQueryContext> TargetContext;

    UPROPERTY(EditDefaultsOnly, Category = "Generator")
    FAIDataProviderIntValue NumSamples;

    UPROPERTY(EditDefaultsOnly, Category = "Generator")
    FAIDataProviderFloatValue MinRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Generator")
    FAIDataProviderFloatValue MaxRadius;

    UPROPERTY(EditDefaultsOnly, Category = "Generator",
        meta = (ClampMin = "5.0", ClampMax = "180.0"))
    FAIDataProviderFloatValue ArcHalfAngleDeg;

    UPROPERTY(EditDefaultsOnly, Category = "Generator|Randomness",
        meta = (ClampMin = "0.0", ClampMax = "45.0"))
    FAIDataProviderFloatValue AngleJitterDeg;

    UPROPERTY(EditDefaultsOnly, Category = "Generator")
    bool bUseFallbackForwardOnZeroVelocity = true;
};