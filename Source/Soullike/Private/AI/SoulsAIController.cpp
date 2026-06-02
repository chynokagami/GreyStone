#include "AI/SoulsAIController.h"

#include "Soullike.h"         // LogGame
#include "AI/BlackboardKeys.h"   // SoulsBBKeys::TargetActor
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "BrainComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"

ASoulsAIController::ASoulsAIController()
{

    AIPerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComp"));

    // Sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius                  = SightRadius;
    SightConfig->LoseSightRadius              = SightLoseRadius;
    SightConfig->PeripheralVisionAngleDegrees = SightPeripheralAngleDeg;
    SightConfig->SetMaxAge(SightMaxAge);

    SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.0f;
    // Souls enemies see hostiles AND neutral (curious enemies stare at NPCs).
    // Friendlies are filtered so allies don't aggro each other.
    SightConfig->DetectionByAffiliation.bDetectEnemies    = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRange;
    HearingConfig->SetMaxAge(SightMaxAge);   // share decay window with sight; tune later if needed
    HearingConfig->DetectionByAffiliation.bDetectEnemies    = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals   = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;

    // Register both senses with the perception component.
    AIPerceptionComp->ConfigureSense(*SightConfig);
    AIPerceptionComp->ConfigureSense(*HearingConfig);
    // Sight wins ties — when a target is both seen and heard, sight's data is canonical.
    AIPerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

    // Bind the perception event. AddDynamic in the constructor is safe for
    // UPROPERTY-owned subobjects: the delegate field is initialized by the
    // GENERATED_BODY scaffolding before the constructor body runs.
    AIPerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &ASoulsAIController::OnPerceptionUpdated);
}

void ASoulsAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!InPawn)
    {
        return;
    }

    if (!DefaultBehaviorTree)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[AIController] %s possessed %s but DefaultBehaviorTree not configured"),
            *GetName(), *GetNameSafe(InPawn));
        return;
    }

    if (DefaultBlackboard)
    {
        UBlackboardComponent* BB = nullptr;
        UseBlackboard(DefaultBlackboard, BB);
    }

    RunBehaviorTree(DefaultBehaviorTree);

    UE_LOG(LogGame, Log, TEXT("[AIController] %s -> possessed %s, BT running"),
        *GetName(), *InPawn->GetName());
}

void ASoulsAIController::OnUnPossess()
{
    GetWorldTimerManager().ClearTimer(LostSightClearTimer);
    GetWorldTimerManager().ClearTimer(NoiseInvestigationClearTimer);
    PendingLostTarget = nullptr;

    if (UBrainComponent* Brain = GetBrainComponent())
    {
        Brain->StopLogic(TEXT("Pawn unpossessed"));
    }

    Super::OnUnPossess();
}

void ASoulsAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    // Dispatch by sense type
    const FAISenseID SightID = UAISense::GetSenseID<UAISense_Sight>();
    const FAISenseID HearingID = UAISense::GetSenseID<UAISense_Hearing>();
    if (Stimulus.Type != SightID)
    {
        if (Stimulus.Type == HearingID && Stimulus.WasSuccessfullySensed())
        {
            BB->SetValueAsVector(SoulsBBKeys::LastNoiseLocation, Stimulus.StimulusLocation);

            GetWorldTimerManager().ClearTimer(NoiseInvestigationClearTimer);
            GetWorldTimerManager().SetTimer(
                NoiseInvestigationClearTimer,
                this,
                &ASoulsAIController::HandleNoiseInvestigationTimerElapsed,
                NoiseInvestigationMemorySeconds,
                /*bLoop*/ false);

            UE_LOG(LogGame, Log,
                TEXT("[AIController] heard noise at %s — investigating for %.1fs"),
                *Stimulus.StimulusLocation.ToCompactString(),
                NoiseInvestigationMemorySeconds);
        }
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        // Sight (re-)acquired
        const bool bHadPendingClear = LostSightClearTimer.IsValid()
            && GetWorldTimerManager().IsTimerActive(LostSightClearTimer);

        if (bHadPendingClear)
        {
            GetWorldTimerManager().ClearTimer(LostSightClearTimer);
            PendingLostTarget = nullptr;

            UE_LOG(LogGame, Log,
                TEXT("[AIController] re-acquired %s during grace window"),
                *Actor->GetName());
        }
        else
        {
            UE_LOG(LogGame, Log, TEXT("[AIController] saw %s"), *Actor->GetName());
        }

        BB->SetValueAsObject(SoulsBBKeys::TargetActor, Actor);
    }
    else
    {
        // ===== Sight lost — start grace period (do NOT clear BB yet) =====
        // Only act if THIS actor is the current BB target. A stale stimulus
        // expiring on a no-longer-tracked actor must not blank a fresh
        // target that just got written by a different sight event.
        if (BB->GetValueAsObject(SoulsBBKeys::TargetActor) != Actor)
        {
            return;
        }

        // Defensive: if a previous grace timer was already running (rapid
        // see -> lose -> see -> lose toggling at FOV boundary), clear it
        // before starting a new one. SetTimer with the same handle would
        // overwrite anyway, but explicit ClearTimer keeps the log honest.
        GetWorldTimerManager().ClearTimer(LostSightClearTimer);

        PendingLostTarget = Actor;

        GetWorldTimerManager().SetTimer(
            LostSightClearTimer,
            this,
            &ASoulsAIController::HandleLostSightTimerElapsed,
            LostSightGracePeriodSeconds,
            /*bLoop*/ false);

        UE_LOG(LogGame, Log,
            TEXT("[AIController] lost sight of %s — grace %.1fs (BB still set)"),
            *Actor->GetName(), LostSightGracePeriodSeconds);
    }
}

void ASoulsAIController::HandleLostSightTimerElapsed()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        // Should not happen — OnUnPossess clears the timer before BB tears
        // down — but defensive return rather than null-deref if it does.
        PendingLostTarget = nullptr;
        return;
    }

    AActor* PendingActor = PendingLostTarget.Get();   // null if invalidated

    if (!PendingActor)
    {
        BB->ClearValue(SoulsBBKeys::TargetActor);
        UE_LOG(LogGame, Log,
            TEXT("[AIController] grace expired (target destroyed) — TargetActor cleared"));
    }
    else if (BB->GetValueAsObject(SoulsBBKeys::TargetActor) == PendingActor)
    {
        BB->ClearValue(SoulsBBKeys::TargetActor);
        UE_LOG(LogGame, Log,
            TEXT("[AIController] grace expired — forgot %s"),
            *PendingActor->GetName());
    }
    else
    {
        UE_LOG(LogGame, Verbose,
            TEXT("[AIController] grace timer fired for %s but BB now holds different target — leaving as-is"),
            *PendingActor->GetName());
    }

    PendingLostTarget = nullptr;
}

void ASoulsAIController::HandleNoiseInvestigationTimerElapsed()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB)
    {
        return;
    }

    BB->ClearValue(SoulsBBKeys::LastNoiseLocation);
    UE_LOG(LogGame, Verbose, TEXT("[AIController] noise investigation expired"));
}
