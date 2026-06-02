#include "Animation/SoulsAnimNotifyState_IFrame.h"

#include "Soullike.h"   // LogGame
#include "ActionSystem/SoulsActionSystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SharedGameplayTags.h"

USoulsAnimNotifyState_IFrame::USoulsAnimNotifyState_IFrame()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(160, 80, 255);   // purple — invulnerability
    bIsNativeBranchingPoint = false;
#endif
}

void USoulsAnimNotifyState_IFrame::NotifyBegin(USkeletalMeshComponent* MeshComp,
                                                UAnimSequenceBase* /*Animation*/,
                                                float /*TotalDuration*/,
                                                const FAnimNotifyEventReference& /*EventReference*/)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (USoulsActionSystemComponent* ASC =
        Owner->FindComponentByClass<USoulsActionSystemComponent>())
    {
        ASC->ActiveGameplayTags.AddTag(SharedGameplayTags::Status_IFrame);
        UE_LOG(LogGame, Log, TEXT("[IFrame] BEGIN on %s"), *Owner->GetName());
    }
}

void USoulsAnimNotifyState_IFrame::NotifyEnd(USkeletalMeshComponent* MeshComp,
                                              UAnimSequenceBase* /*Animation*/,
                                              const FAnimNotifyEventReference& /*EventReference*/)
{
    if (!MeshComp) return;
    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (USoulsActionSystemComponent* ASC =
        Owner->FindComponentByClass<USoulsActionSystemComponent>())
    {
        ASC->ActiveGameplayTags.RemoveTag(SharedGameplayTags::Status_IFrame);
        UE_LOG(LogGame, Log, TEXT("[IFrame] END on %s"), *Owner->GetName());
    }
}