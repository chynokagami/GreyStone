#include "AI/BTDecorators/SoulsBTDecorator_IsLowHealth.h"

#include "Soullike.h"                                  // LogGame
#include "SharedGameplayTags.h"                        // SharedGameplayTags::Attribute_*
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

USoulsBTDecorator_IsLowHealth::USoulsBTDecorator_IsLowHealth()
{
    NodeName = TEXT("Is Low Health");
}

bool USoulsBTDecorator_IsLowHealth::CalculateRawConditionValue(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    // Resolve owning pawn
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!ensure(AICon))
    {
        return false;
    }

    APawn* OwningPawn = AICon->GetPawn();
    if (!OwningPawn)
    {
        return false;
    }

    // Resolve ASC
    USoulsActionSystemComponent* ASC =
        OwningPawn->FindComponentByClass<USoulsActionSystemComponent>();
    if (!ensure(ASC))
    {
        // Misconfigured pawn: AI without ASC. Log once via ensure, fall through.
        return false;
    }

    // Compute health fraction
    const float HealthMax = ASC->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);
    if (HealthMax <= KINDA_SMALL_NUMBER)
    {
        // Pawn has no HealthMax configured (or zeroed by buff). Avoid div-by-zero.
        // Treat as "low health" so a Flee branch can still trigger if designers
        // intentionally drain HealthMax to 0 (rare).
        return true;
    }

    const float Health = ASC->GetAttributeValue(SharedGameplayTags::Attribute_Health);
    const float HealthFraction = Health / HealthMax;

    return HealthFraction < LowHealthFraction;
}

FString USoulsBTDecorator_IsLowHealth::GetStaticDescription() const
{
    // Rendered as a sub-line under NodeName in the BT editor.
    // Format: "Is Health < 30%". Designers tweak LowHealthFraction and see the
    // updated threshold immediately on the node — no Details-panel hunting.
    return FString::Printf(TEXT("Is Health < %.0f%%"), LowHealthFraction * 100.0f);
}