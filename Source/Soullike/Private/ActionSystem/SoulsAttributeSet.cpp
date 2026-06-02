#include "ActionSystem/SoulsAttributeSet.h"

#include "ActionSystem/SoulsActionSystemComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"



USoulsActionSystemComponent* USoulsAttributeSet::GetOwningComponent() const
{
    return Cast<USoulsActionSystemComponent>(GetOuter());
}



namespace
{
    
    void ClampAttribute(FSoulsAttribute& Attr, float MaxValue)
    {
        const float Current = Attr.GetValue();
        if (Current < 0.f)
        {
            Attr.Modifier = -Attr.Base;
        }
        else if (Current > MaxValue)
        {
            Attr.Modifier = MaxValue - Attr.Base;
        }
    }
}



USoulsHealthAttributeSet::USoulsHealthAttributeSet()
{
    Health = FSoulsAttribute(100.f);
    HealthMax = FSoulsAttribute(Health.GetValue());
}

void USoulsHealthAttributeSet::PostAttributeChanged()
{
    ClampAttribute(Health, HealthMax.GetValue());
}



USoulsPawnAttributeSet::USoulsPawnAttributeSet()
{
    MoveSpeed = FSoulsAttribute(550.f);
    MoveSpeedMultiplier = FSoulsAttribute(1.0f);

    Stamina = FSoulsAttribute(100.f);
    StaminaMax = FSoulsAttribute(100.f);
    StaminaRegenRate = FSoulsAttribute(45.f);
    StaminaRegenDelay = FSoulsAttribute(1.0f);

    Posture = FSoulsAttribute(0.f);
    PostureMax = FSoulsAttribute(100.f);

    Poise = FSoulsAttribute(100.f);
    PoiseMax = FSoulsAttribute(100.f);

    Defense = FSoulsAttribute(0.f);
}

void USoulsPawnAttributeSet::PostAttributeChanged()
{
    Super::PostAttributeChanged();

    ClampAttribute(Stamina, StaminaMax.GetValue());
    ClampAttribute(Posture, PostureMax.GetValue());
    ClampAttribute(Poise, PoiseMax.GetValue());

    ApplyMoveSpeed();
}

void USoulsPawnAttributeSet::InitializeAttributes()
{
    Super::InitializeAttributes();
    ApplyMoveSpeed();
}

void USoulsPawnAttributeSet::ApplyMoveSpeed()
{
    USoulsActionSystemComponent* Comp = GetOwningComponent();
    if (!Comp) return;

    ACharacter* OwningCharacter = Cast<ACharacter>(Comp->GetOwner());
    if (!OwningCharacter || !OwningCharacter->GetCharacterMovement()) return;

    OwningCharacter->GetCharacterMovement()->MaxWalkSpeed =
        MoveSpeed.GetValue() * MoveSpeedMultiplier.GetValue();
}



USoulsPlayerAttributeSet::USoulsPlayerAttributeSet()
{
    FlaskCharges = FSoulsAttribute(5.f);
    FocusPoints = FSoulsAttribute(0.f);
}



USoulsMonsterAttributeSet::USoulsMonsterAttributeSet()
{
    Health = FSoulsAttribute(1500.f);
    HealthMax = FSoulsAttribute(1500.f);
    MoveSpeed = FSoulsAttribute(650.f);
    SoulsReward = FSoulsAttribute(50.f);
}