#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryContext.h"
#include "SoulsEnvQueryContext_TargetActor.generated.h"

/**
 * EQS Context that resolves to "the AI's current target actor" — pulled
 * from the AIController's Blackboard via BlackboardKeys::TargetActor.
 *
 * What is an EnvQueryContext?
 *   EQS is UE's spatial query system: "find the best point that satisfies X
 *   conditions". Most queries need a REFERENCE POINT to test "near/far from
 *   what". That reference is called a Context. Engine ships:
 *     - EnvQueryContext_Querier   = the actor running the query (the AI)
 *     - EnvQueryContext_Item      = each candidate point being scored
 *   Custom Contexts (like this one) extend the dictionary of reference points
 *   the EQS asset can use in its conditions.
 *
 * Why this Context exists:
 *   Without it, EQS conditions like "score points by distance from
 *   TargetActor" can't be expressed — there's no built-in way for the EQS
 *   asset to know which actor is "the target". This Context bridges
 *   BehaviorTree's Blackboard (TargetActor key) into EQS-land.
 *
 * Used by (sub-item 13 and beyond):
 *   - EQS_FlankPoint: "find a point behind TargetActor"
 *   - EQS_Retreat:    "find a point far from TargetActor" (P3+)
 *   - EQS_AttackArc:  "find a point in 180° front of TargetActor" (P3+)
 *
 * Architectural note:
 *   This is a UEnvQueryContext UObject — engine instantiates exactly ONE
 *   per game (it's stateless, ProvideContext re-resolves every query).
 *   No member fields, no per-instance state. Hence we don't TObjectPtr
 *   anything; ContextData is engine-allocated each call.
 *
 */
UCLASS()
class SOULLIKE_API USoulsEnvQueryContext_TargetActor : public UEnvQueryContext
{
    GENERATED_BODY()

    /**
     * Engine entry point — fills ContextData with the actor(s) this context
     * represents. Called once per EQS query that references this context.
     *
     * Resolution chain:
     *   QueryInstance.Owner = querier pawn (the AI running the query)
     *     -> Pawn->GetController() = AAIController
     *     -> Controller->GetBlackboardComponent()->GetValueAsObject(TargetActor)
     *     -> Cast to AActor
     *     -> SetContextHelper packs it into ContextData
     */
    virtual void ProvideContext(FEnvQueryInstance& QueryInstance,
                                FEnvQueryContextData& ContextData) const override;
};