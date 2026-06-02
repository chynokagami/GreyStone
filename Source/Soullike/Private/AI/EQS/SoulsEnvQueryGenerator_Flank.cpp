#include "AI/EQS/SoulsEnvQueryGenerator_Flank.h"

#include "Soullike.h"                                          // LogGame

#include "AIController.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

USoulsEnvQueryGenerator_Flank::USoulsEnvQueryGenerator_Flank()
{
    ItemType = UEnvQueryItemType_Point::StaticClass();

    TargetContext = UEnvQueryContext_Querier::StaticClass();

    NumSamples.DefaultValue        = 12;       
    MinRadius.DefaultValue         = 200.0f;   
    MaxRadius.DefaultValue         = 600.0f;   
    ArcHalfAngleDeg.DefaultValue   = 60.0f;    
    AngleJitterDeg.DefaultValue    = 12.0f;    

    // bUseFallbackForwardOnZeroVelocity = true
}

void USoulsEnvQueryGenerator_Flank::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
    // Resolve TargetContext to actor list
    TArray<AActor*> TargetActors;
    if (!QueryInstance.PrepareContext(TargetContext, TargetActors) ||
        TargetActors.Num() == 0)
    {
        // No target → no points. EQS pipeline handles "0 items" gracefully
        // (BT MoveTo task will fail this branch, BT picks next branch).
        UE_LOG(LogGame, Log,
            TEXT("[Flank] No target actor — generating 0 points"));
        return;
    }

    // We only handle the first target. If a future use-case wants
    // multi-target flanking (e.g. flank between two enemies), this is the
    // hook to expand.
    AActor* Target = TargetActors[0];
    if (!Target)
    {
        return;
    }

    const FVector TargetLoc = Target->GetActorLocation();

    // Resolve target's forward direction
    // Use only horizontal components — flank is a ground plane operation,
    // we don't want vertical tilt to skew the arc.
    FVector ForwardXY = Target->GetActorForwardVector();
    ForwardXY.Z = 0.0f;


    if (ForwardXY.IsNearlyZero())
    {
        if (bUseFallbackForwardOnZeroVelocity)
        {
            // Degenerate target forward → use the direction from Querier to
            // Target as a proxy. This treats "behind" as "the side of Target
            // away from where the AI is approaching from", which is a better
            // semantic default than an arbitrary world axis.
            //
            // Falls back to world +X only if Querier itself is degenerate
            // (e.g. EQS Test Pawn in editor with no controller).
            if (APawn* Querier = Cast<APawn>(QueryInstance.Owner.Get()))
            {
                FVector QuerierToTarget = TargetLoc - Querier->GetActorLocation();
                QuerierToTarget.Z = 0.0f;
                if (!QuerierToTarget.IsNearlyZero())
                {
                    ForwardXY = QuerierToTarget.GetSafeNormal();
                    UE_LOG(LogGame, Log,
                        TEXT("[Flank] %s has degenerate forward — using Querier->Target direction"),
                        *Target->GetName());
                }
                else
                {
                    ForwardXY = FVector(1.0f, 0.0f, 0.0f);
                    UE_LOG(LogGame, Log,
                        TEXT("[Flank] %s has degenerate forward AND querier overlaps target — using world +X"),
                        *Target->GetName());
                }
            }
            else
            {
                ForwardXY = FVector(1.0f, 0.0f, 0.0f);
                UE_LOG(LogGame, Log,
                    TEXT("[Flank] %s has degenerate forward, no querier pawn — using world +X"),
                    *Target->GetName());
            }
        }
        else

        {
            UE_LOG(LogGame, Verbose,
                TEXT("[Flank] %s has degenerate forward, fallback disabled — generating 0 points"),
                *Target->GetName());
            return;
        }
    }
    ForwardXY.Normalize();

    // Bind FAIDataProvider*Value fields
    NumSamples.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
    MinRadius.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
    MaxRadius.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
    ArcHalfAngleDeg.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);
    AngleJitterDeg.BindData(QueryInstance.Owner.Get(), QueryInstance.QueryID);

    const int32 N         = FMath::Max(1, NumSamples.GetValue());
    const float MinR      = FMath::Max(0.0f, MinRadius.GetValue());
    const float MaxR      = FMath::Max(MinR + 1.0f, MaxRadius.GetValue());  // ensure Max > Min
    const float ArcHalf   = FMath::Clamp(ArcHalfAngleDeg.GetValue(), 5.0f, 180.0f);
    const float Jitter    = FMath::Clamp(AngleJitterDeg.GetValue(), 0.0f, 45.0f);

    // Build sample points
    const FVector BackAxisXY = -ForwardXY;

    // Reserve generated point list (parent class manages the actual array
    // through GenericItems → AddItem).
    TArray<FNavLocation> RawPoints;
    RawPoints.Reserve(N);

    for (int32 i = 0; i < N; ++i)
    {
        // Base angle uniform in [-ArcHalf, +ArcHalf] (degrees, around
        // the back axis). Plus jitter.
        const float BaseAngleDeg   = FMath::FRandRange(-ArcHalf, ArcHalf);
        const float JitterAngleDeg = FMath::FRandRange(-Jitter, Jitter);
        const float TotalAngleDeg  = BaseAngleDeg + JitterAngleDeg;

        // Rotate BackAxisXY by TotalAngleDeg around world Z. RotateAngleAxis
        // takes degrees + axis vector — UE convention.
        const FVector Direction = BackAxisXY.RotateAngleAxis(
            TotalAngleDeg, FVector::UpVector);

        // Radius uniform in band.
        const float Radius = FMath::FRandRange(MinR, MaxR);

        // Build world point. Z = target Z; NavMesh projection (parent class)
        // will clamp Z to the actual nav surface.
        const FVector RawPoint = TargetLoc + Direction * Radius;
        RawPoints.Add(FNavLocation(RawPoint));
    }

    // Snapshot raw count BEFORE projection mutates the array. Project will
    // shrink RawPoints in-place by removing samples that don't hit NavMesh.
    const int32 RawCount = RawPoints.Num();

    ProjectAndFilterNavPoints(RawPoints, QueryInstance);
    StoreNavPoints(RawPoints, QueryInstance);

    const int32 Survived = RawPoints.Num();

    // Diagnostic logging
    if (Survived == 0)
    {
        UE_LOG(LogGame, Warning,
            TEXT("[Flank] %d raw samples around %s — ALL FAILED NavMesh projection. "
                "Check NavMeshBoundsVolume coverage and Project Down distance."),
            RawCount, *Target->GetName());
    }
    else if (Survived * 2 < RawCount)
    {
        UE_LOG(LogGame, Log,
            TEXT("[Flank] %d raw samples around %s, only %d survived NavMesh projection (<50%%)"),
            RawCount, *Target->GetName(), Survived);
    }
    else
    {
        UE_LOG(LogGame, Verbose,
            TEXT("[Flank] Generated %d raw samples around %s, %d survived NavMesh projection"),
            RawCount, *Target->GetName(), Survived);
    }

}

FText USoulsEnvQueryGenerator_Flank::GetDescriptionTitle() const
{
    return FText::FromString(TEXT("Souls Flank Point"));
}

FText USoulsEnvQueryGenerator_Flank::GetDescriptionDetails() const
{
    // Shown beneath the node title in EQS editor — gives designers a
    // one-line summary of current config without opening the details panel.
    return FText::FromString(FString::Printf(
        TEXT("Sample %d points behind target, %0.f-%0.f cm, ±%0.f° arc, ±%0.f° jitter"),
        NumSamples.GetValue(),
        MinRadius.GetValue(),
        MaxRadius.GetValue(),
        ArcHalfAngleDeg.GetValue(),
        AngleJitterDeg.GetValue()));
}