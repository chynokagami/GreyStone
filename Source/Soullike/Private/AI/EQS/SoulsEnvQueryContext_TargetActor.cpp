#include "AI/EQS/SoulsEnvQueryContext_TargetActor.h"

#include "Soullike.h"                                          // LogGame
#include "AI/BlackboardKeys.h"                                 // BlackboardKeys::TargetActor

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

void USoulsEnvQueryContext_TargetActor::ProvideContext(
    FEnvQueryInstance& QueryInstance, FEnvQueryContextData& ContextData) const
{
    // Resolve querier pawn
    APawn* QuerierPawn = Cast<APawn>(QueryInstance.Owner.Get());
    if (!QuerierPawn)
    {
        // Should not happen in BT context; guard against editor preview /
        // standalone EQS test where Owner might be a non-Pawn.
        UE_LOG(LogGame, Verbose,
            TEXT("[EQS_TargetActor] Querier is not a Pawn — context empty"));
        return;
    }

    // Resolve AIController
    AAIController* Controller = Cast<AAIController>(QuerierPawn->GetController());
    if (!Controller)
    {
        UE_LOG(LogGame, Verbose,
            TEXT("[EQS_TargetActor] %s has no AIController — context empty"),
            *QuerierPawn->GetName());
        return;
    }

    UBlackboardComponent* BB = Controller->GetBlackboardComponent();
    if (!BB)
    {
        UE_LOG(LogGame, Verbose,
            TEXT("[EQS_TargetActor] %s has no Blackboard — context empty"),
            *QuerierPawn->GetName());
        return;
    }

    // Pull TargetActor from Blackboard
    AActor* TargetActor =
        Cast<AActor>(BB->GetValueAsObject(SoulsBBKeys::TargetActor));

    // Pack into ContextData
    UEnvQueryItemType_Actor::SetContextHelper(ContextData, TargetActor);

    UE_LOG(LogGame, VeryVerbose,
        TEXT("[EQS_TargetActor] %s -> target = %s"),
        *QuerierPawn->GetName(),
        TargetActor ? *TargetActor->GetName() : TEXT("(none)"));
}