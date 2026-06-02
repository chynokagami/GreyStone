#pragma once

#include "CoreMinimal.h"

namespace SoulsBBKeys
{
    // Combat target tracking
    /** UObject (Actor*). Set by AIPerception sight events. nullptr = no target. */
    extern SOULLIKE_API const FName TargetActor;

    // Hearing (sub-item 3+; Vector noise position)
    /** Vector. Set when a hearing stimulus fires. AI investigates if no visual target. */
    extern SOULLIKE_API const FName LastNoiseLocation;

    // BTService_CheckRangeTo writes
    /** Bool. True when distance to TargetActor < AttackRadius. */
    extern SOULLIKE_API const FName bIsInAttackRange;

    /** Bool. True when AI can see TargetActor (LineOfSight raycast hit clear). */
    extern SOULLIKE_API const FName bHasLineOfSight;

    // Combat context / intent
    /** Float. Current distance from AI pawn to TargetActor in cm. */
    extern SOULLIKE_API const FName DistanceToTarget;

    /** Float. Absolute angle from AI forward to target direction in degrees. */
    extern SOULLIKE_API const FName TargetAngleAbs;

    /** Float. Owning pawn health divided by max health, clamped to [0, 1]. */
    extern SOULLIKE_API const FName HealthRatio;

    /** Bool. True when the current frame is a good melee attack opportunity. */
    extern SOULLIKE_API const FName bShouldAttack;

    /** Bool. True when AI should orbit/strafe instead of standing still. */
    extern SOULLIKE_API const FName bShouldStrafe;

    /** Bool. True when AI should look for a side/back reposition point. */
    extern SOULLIKE_API const FName bShouldFlank;

    /** Bool. True when AI should disengage and create space. */
    extern SOULLIKE_API const FName bShouldRetreat;

    // Patrol
    /** Vector. The pawn's spawn location, used as patrol anchor / leash center. */
    extern SOULLIKE_API const FName HomeLocation;
}
