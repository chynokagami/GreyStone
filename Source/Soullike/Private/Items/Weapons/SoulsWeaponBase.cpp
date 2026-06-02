#include "Items/Weapons/SoulsWeaponBase.h"

#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Components/BoxComponent.h"
#include "Core/SoulsHitInterface.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

#include "Soullike.h"  // LogGame
#include "Characters/SoulsBaseCharacter.h"
#include "Characters/SoulsPlayerCharacter.h"
#include "SharedGameplayTags.h"

ASoulsWeaponBase::ASoulsWeaponBase()
{
    PrimaryActorTick.bCanEverTick = false;

    WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox"));
    SetRootComponent(WeaponBox);

    WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponBox->SetGenerateOverlapEvents(false);

    BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceStart"));
    BoxTraceStart->SetupAttachment(WeaponBox);

    BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("BoxTraceEnd"));
    BoxTraceEnd->SetupAttachment(WeaponBox);
}

void ASoulsWeaponBase::BeginPlay()
{
    Super::BeginPlay();
}

void ASoulsWeaponBase::Equip(USceneComponent* InParent, FName InSocketName,
    AActor* NewOwner, APawn* NewInstigator)
{
    if (!InParent)
    {
        UE_LOG(LogGame, Warning, TEXT("[Equip] %s: InParent is null"), *GetName());
        return;
    }

    SetOwner(NewOwner);
    SetInstigator(NewInstigator);

    // SnapToTarget for Location+Rotation so socket transform fully governs placement.
    // KeepWorld for Scale so weapon scale isn't disturbed by skeleton scaling.
    const FAttachmentTransformRules Rules(
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::SnapToTarget,
        EAttachmentRule::KeepWorld,
        /*bWeldSimulatedBodies=*/ true);

    AttachToComponent(InParent, Rules, InSocketName);

    UE_LOG(LogGame, Log, TEXT("[Equip] %s -> %s socket on %s"),
        *GetName(), *InSocketName.ToString(),
        NewOwner ? *NewOwner->GetName() : TEXT("(null owner)"));
}

void ASoulsWeaponBase::BeginHitDetection(float InDamage, FGameplayTag InDamageType)
{
    CurrentDamage = InDamage;
    if (const ASoulsPlayerCharacter* PlayerOwner = Cast<ASoulsPlayerCharacter>(GetOwner()))
    {
        if (PlayerOwner->IsHeavyChargeDamageActive())
        {
            CurrentDamage *= PlayerOwner->GetHeavyChargeDamageMultiplier();
        }
    }

    CurrentDamageType = InDamageType;
    IgnoredActors.Reset();
    bHitDetectionActive = true;
    GetTraceSamples(PreviousFrameTraceSamples);

    UE_LOG(LogGame, Verbose, TEXT("[Weapon] BeginHitDetection dmg=%.0f base=%.0f type=%s"),
        CurrentDamage, InDamage, InDamageType.IsValid() ? *InDamageType.ToString() : TEXT("(none)"));
}

void ASoulsWeaponBase::UpdateHitDetection(float /*DeltaTime*/)
{
    if (!bHitDetectionActive || !GetWorld() || !BoxTraceStart || !BoxTraceEnd)
    {
        return;
    }

    TArray<FVector> CurrentSamples;
    GetTraceSamples(CurrentSamples);
    if (PreviousFrameTraceSamples.Num() != CurrentSamples.Num())
    {
        PreviousFrameTraceSamples = CurrentSamples;
        return;
    }

    FCollisionQueryParams QueryParams(TEXT("SoulsWeaponTrace"), false);
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(this);
    for (const TObjectPtr<AActor>& IgnoredActor : IgnoredActors)
    {
        QueryParams.AddIgnoredActor(IgnoredActor.Get());
    }

    const FCollisionShape TraceShape = FCollisionShape::MakeBox(BoxTraceExtent);
    const FQuat TraceRotation = BoxTraceStart->GetComponentQuat();
    TArray<FHitResult> HitResults;

    for (int32 Index = 0; Index < CurrentSamples.Num(); ++Index)
    {
        FVector Start = PreviousFrameTraceSamples[Index];
        const FVector End = CurrentSamples[Index];
        if (Start.Equals(End))
        {
            Start += FVector::UpVector * UE_KINDA_SMALL_NUMBER;
        }

        HitResults.Reset();
        const bool bHit = GetWorld()->SweepMultiByChannel(
            HitResults,
            Start,
            End,
            TraceRotation,
            TraceChannel,
            TraceShape,
            QueryParams);

        if (bShowBoxDebug)
        {
            const FColor DebugColor = bHit ? FColor::Red : FColor::Green;
            DrawDebugBox(GetWorld(), End, BoxTraceExtent, TraceRotation, DebugColor, false, 0.1f);
            DrawDebugLine(GetWorld(), Start, End, DebugColor, false, 0.1f, 0, 1.f);
        }

        if (!bHit)
        {
            continue;
        }

        for (const FHitResult& HitResult : HitResults)
        {
            HandleTraceHit(HitResult);
            if (AActor* HitActor = HitResult.GetActor())
            {
                QueryParams.AddIgnoredActor(HitActor);
            }
        }
    }

    PreviousFrameTraceSamples = MoveTemp(CurrentSamples);
}
void ASoulsWeaponBase::EndHitDetection()
{
    bHitDetectionActive = false;
    CurrentDamage = 0.f;
    CurrentDamageType = FGameplayTag();
    PreviousFrameTraceSamples.Reset();
    IgnoredActors.Reset();

    UE_LOG(LogGame, Verbose, TEXT("[Weapon] EndHitDetection"));
}

void ASoulsWeaponBase::GetTraceSamples(TArray<FVector>& OutSamples) const
{
    OutSamples.Reset(TraceDensity + 1);
    if (!BoxTraceStart || !BoxTraceEnd)
    {
        return;
    }

    const int32 SafeDensity = FMath::Max(TraceDensity, 1);
    const FVector Start = BoxTraceStart->GetComponentLocation();
    const FVector End = BoxTraceEnd->GetComponentLocation();
    for (int32 Index = 0; Index <= SafeDensity; ++Index)
    {
        const float Alpha = static_cast<float>(Index) / static_cast<float>(SafeDensity);
        OutSamples.Add(FMath::Lerp(Start, End, Alpha));
    }
}

void ASoulsWeaponBase::HandleTraceHit(const FHitResult& HitResult)
{
    AActor* HitActor = HitResult.GetActor();
    if (!HitActor) return;
    if (HitActor == GetOwner()) return;
    if (IsSameTeam(HitActor)) return;
    if (IgnoredActors.Contains(HitActor)) return;

    IgnoredActors.AddUnique(HitActor);
    ExecuteHit(HitResult);
}

bool ASoulsWeaponBase::IsSameTeam(const AActor* OtherActor) const
{
    if (!OtherActor) return false;

    const ASoulsBaseCharacter* WielderChar = Cast<ASoulsBaseCharacter>(GetOwner());
    const ASoulsBaseCharacter* TargetChar = Cast<ASoulsBaseCharacter>(OtherActor);

    // Tagless actors (props, breakables, debug dummies) are always damageable.
    // This is intentional: makes B6 self-hit testing work without configuring tags on test BPs.
    if (!WielderChar || !TargetChar) return false;
    if (!WielderChar->TeamTag.IsValid() || !TargetChar->TeamTag.IsValid()) return false;

    return WielderChar->TeamTag == TargetChar->TeamTag;
}

void ASoulsWeaponBase::ExecuteHit(const FHitResult& BoxHit)
{
    AActor* HitActor = BoxHit.GetActor();
    if (!HitActor) return;

    AController* InstigatorController = GetInstigator() ? GetInstigator()->GetController() : nullptr;

    // Damage routes through engine -> target's TakeDamage override -> ASC.
    UGameplayStatics::ApplyDamage(
        HitActor,
        CurrentDamage,
        InstigatorController,
        this,                          // DamageCauser = the weapon
        UDamageType::StaticClass());

    // Visual reaction. Ordering matches DebugTakeHit:
    // GetHit before damage settles -> Stun added, HitReact starts; if lethal,
    // Die() interrupts. ApplyDamage above already kicked off the chain.
    if (HitActor->Implements<USoulsHitInterface>())
    {
        ISoulsHitInterface::Execute_GetHit(HitActor, BoxHit.ImpactPoint, GetOwner());
    }

    UE_LOG(LogGame, Log, TEXT("[Weapon] %s hit %s for %.0f dmg (%s)"),
        *GetName(), *HitActor->GetName(), CurrentDamage,
        CurrentDamageType.IsValid() ? *CurrentDamageType.ToString() : TEXT("Default"));
}
