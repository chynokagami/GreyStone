#include "Animation/SoulsAnimNotify_ResetCombo.h"

#include "Characters/SoulsPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

USoulsAnimNotify_ResetCombo::USoulsAnimNotify_ResetCombo()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(255, 180, 80);  // orange, end-of-attack marker
    bIsNativeBranchingPoint = false;
#endif
}

void USoulsAnimNotify_ResetCombo::Notify(USkeletalMeshComponent* MeshComp,
                                          UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(Owner))
    {
        Player->TryAdvanceComboOrReset();
    }
}