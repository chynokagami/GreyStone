#include "Animation/SoulsAnimNotify_SaveAttack.h"

#include "Characters/SoulsPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

USoulsAnimNotify_SaveAttack::USoulsAnimNotify_SaveAttack()
{
#if WITH_EDITORONLY_DATA
    NotifyColor = FColor(80, 180, 255);  // light blue, distinct from DamageWindow green
    bIsNativeBranchingPoint = false;
#endif
}

void USoulsAnimNotify_SaveAttack::Notify(USkeletalMeshComponent* MeshComp,
                                          UAnimSequenceBase* Animation,
                                          const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp) return;

    AActor* Owner = MeshComp->GetOwner();
    if (!Owner) return;

    if (ASoulsPlayerCharacter* Player = Cast<ASoulsPlayerCharacter>(Owner))
    {
        Player->EnableComboInput();
    }
}