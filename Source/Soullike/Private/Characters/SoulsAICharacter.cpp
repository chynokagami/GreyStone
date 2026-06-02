#include "Characters/SoulsAICharacter.h"

#include "Soullike.h"                      // LogGame
#include "SharedGameplayTags.h"            // SharedGameplayTags::Team_Enemy
#include "AIController.h"                  // AAIController, GetController cast
#include "BrainComponent.h"                // UBrainComponent::StopLogic

ASoulsAICharacter::ASoulsAICharacter()
{
    // Default team. Subclasses (or BP CDO) can override if needed
    // (e.g. a "charmed" enemy archetype switching to Team.Player at runtime).
    TeamTag = SharedGameplayTags::Team_Enemy;

    // AI characters do not need per-frame Tick by default.
    // Combat decisions live in BehaviorTree (sub-item 7).
    PrimaryActorTick.bCanEverTick = false;
}

void ASoulsAICharacter::Die()
{
    // Run base lethal handler first:
    //   - sets Status.IsDead tag
    //   - plays DeathMontage
    //   - broadcasts OnDeath
    //   - disables mesh collision
    Super::Die();

    // Stop the BehaviorTree / brain. Without this, the BT keeps ticking on a
    // dead pawn and may try to play attack animations on top of the death pose.
    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        if (UBrainComponent* Brain = AICon->GetBrainComponent())
        {
            Brain->StopLogic(TEXT("Pawn died"));
        }
    }

    SetLifeSpan(CorpseLifetime);

    UE_LOG(LogGame, Log, TEXT("[AICharacter] %s died, corpse lifetime %.1fs"),
        *GetName(), CorpseLifetime);
}