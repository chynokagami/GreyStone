#include "HUD/SoulsHealthBarComponent.h"

#include "Soullike.h"                                          // LogGame
#include "SharedGameplayTags.h"                                // SharedGameplayTags::Attribute_*, Status_IsDead
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Characters/SoulsBaseCharacter.h"
#include "HUD/SoulsHealthBar.h"

#include "Components/ProgressBar.h"                            // SetPercent
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

USoulsHealthBarComponent::USoulsHealthBarComponent()
{
    // Tick configuration
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
    PrimaryComponentTick.bStartWithTickEnabled = true;

    // Widget space
    // World Space — required so we can yaw-billboard manually instead of
    // letting engine pitch-align (which flips bar away when player looks up).
    SetWidgetSpace(EWidgetSpace::World);
    SetDrawSize(FVector2D(200.0f, 30.0f));
    SetPivot(FVector2D(0.5f, 1.0f));

    SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetGenerateOverlapEvents(false);
    bReceivesDecals = false;
}

void USoulsHealthBarComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Resolve ASC
    USoulsActionSystemComponent* ASC =
        Owner->FindComponentByClass<USoulsActionSystemComponent>();
    if (!ensure(ASC))
    {
        UE_LOG(LogGame, Warning,
            TEXT("[HealthBar] %s has no ActionSystemComponent — bar inactive"),
            *Owner->GetName());
        return;
    }

    CachedASC = ASC;

    // Subscribe to Health attribute changes
    HealthListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
        .AddUObject(this, &USoulsHealthBarComponent::HandleHealthChanged);
    HealthMaxListenerHandle = ASC->GetAttributeListener(SharedGameplayTags::Attribute_HealthMax)
        .AddUObject(this, &USoulsHealthBarComponent::HandleHealthChanged);

    // Subscribe to Owner's death broadcast
    if (ASoulsBaseCharacter* AsBase = Cast<ASoulsBaseCharacter>(Owner))
    {
        AsBase->OnDeath.AddDynamic(this, &USoulsHealthBarComponent::HandleOwnerDeath);
    }

    // Initial render pass
    RefreshFromASC();
}

void USoulsHealthBarComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (HealthListenerHandle.IsValid())
    {
        if (USoulsActionSystemComponent* ASC = CachedASC.Get())
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
               .Remove(HealthListenerHandle);
        }
        HealthListenerHandle.Reset();
    }
    if (HealthMaxListenerHandle.IsValid())
    {
        if (USoulsActionSystemComponent* ASC = CachedASC.Get())
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_HealthMax)
               .Remove(HealthMaxListenerHandle);
        }
        HealthMaxListenerHandle.Reset();
    }


    Super::EndPlay(EndPlayReason);
}

void USoulsHealthBarComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Yaw-only billboard
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->PlayerCameraManager)
    {
        return;
    }

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const FVector MyLoc = GetComponentLocation();

    FVector ToCam = CamLoc - MyLoc;
    ToCam.Z = 0.0f;
    if (ToCam.IsNearlyZero())
    {
        return;
    }

    SetWorldRotation(ToCam.Rotation());
}

USoulsHealthBar* USoulsHealthBarComponent::ResolveWidget()
{
    if (CachedWidget)
    {
        return CachedWidget;
    }

    UUserWidget* RawWidget = GetUserWidgetObject();
    if (!RawWidget)
    {
        return nullptr;
    }

    CachedWidget = Cast<USoulsHealthBar>(RawWidget);
    if (!CachedWidget)
    {
        ensureMsgf(false,
            TEXT("[HealthBar] WidgetClass on %s is not USoulsHealthBar subclass"),
            *GetOwner()->GetName());
    }
    return CachedWidget;
}

void USoulsHealthBarComponent::RefreshFromASC()
{
    USoulsActionSystemComponent* ASC = CachedASC.Get();
    if (!ASC)
    {
        return;
    }

    USoulsHealthBar* HealthBarWidget = ResolveWidget();
    if (!HealthBarWidget || !HealthBarWidget->HealthBarProgress)
    {
        return;
    }

    const float Health = ASC->GetAttributeValue(SharedGameplayTags::Attribute_Health);
    const float HealthMax = ASC->GetAttributeValue(SharedGameplayTags::Attribute_HealthMax);

    const float Percent = (HealthMax > KINDA_SMALL_NUMBER)
        ? FMath::Clamp(Health / HealthMax, 0.0f, 1.0f)
        : 0.0f;

    HealthBarWidget->HealthBarProgress->SetPercent(Percent);

    // Visibility
    const bool bIsDead = ASC->ActiveGameplayTags.HasTag(SharedGameplayTags::Status_IsDead);
    const bool bIsFull = bHideAtFullHealth && Percent >= 0.999f;

    SetVisibility(!bIsDead && !bIsFull);
}

void USoulsHealthBarComponent::HandleHealthChanged(
    FGameplayTag /*AttributeTag*/, float /*NewValue*/, float /*OldValue*/)
{
    // Don't use NewValue directly — HealthMax might change in the same frame
    // (buff tick), and we want Health/HealthMax ratio to be self-consistent.
    RefreshFromASC();
}

void USoulsHealthBarComponent::HandleOwnerDeath(ASoulsBaseCharacter* /*DeadCharacter*/)
{
    SetVisibility(false);
    PrimaryComponentTick.SetTickFunctionEnable(false);

    // Unbind health listener early — corpse stays in world 8s but its HP
    // shouldn't change (and if it does, shouldn't update the now-hidden bar).
    if (HealthListenerHandle.IsValid())
    {
        if (USoulsActionSystemComponent* ASC = CachedASC.Get())
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_Health)
               .Remove(HealthListenerHandle);
        }
        HealthListenerHandle.Reset();
    }
    if (HealthMaxListenerHandle.IsValid())
    {
        if (USoulsActionSystemComponent* ASC = CachedASC.Get())
        {
            ASC->GetAttributeListener(SharedGameplayTags::Attribute_HealthMax)
               .Remove(HealthMaxListenerHandle);
        }
        HealthMaxListenerHandle.Reset();
    }
}
