#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SoulsAttributeSet.generated.h"


class USoulsActionSystemComponent;



USTRUCT()
struct FSoulsAttribute
{
    GENERATED_BODY()

    FSoulsAttribute() {}

    FSoulsAttribute(float InBase)
        : Base(InBase) {}

    UPROPERTY(EditAnywhere)
    float Base = 0.0f;

    UPROPERTY(Transient)
    float Modifier = 0.0f;

    float GetValue() const
    {
        return Base + Modifier;
    }
};


UCLASS(EditInlineNew)
class SOULLIKE_API USoulsAttributeSet : public UObject
{
    GENERATED_BODY()

public:

    
    USoulsActionSystemComponent* GetOwningComponent() const;

    
    virtual void InitializeAttributes() {};

    
    virtual void PostAttributeChanged() {};
};



UCLASS()
class SOULLIKE_API USoulsHealthAttributeSet : public USoulsAttributeSet
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=Health)
    FSoulsAttribute Health;

    UPROPERTY(EditAnywhere, Category=Health)
    FSoulsAttribute HealthMax;

    virtual void PostAttributeChanged() override;

    USoulsHealthAttributeSet();
};



UCLASS()
class SOULLIKE_API USoulsPawnAttributeSet : public USoulsHealthAttributeSet
{
    GENERATED_BODY()

public:

    
    UPROPERTY(EditAnywhere, Category=MoveSpeed)
    FSoulsAttribute MoveSpeed;

    UPROPERTY(EditAnywhere, Category=MoveSpeed)
    FSoulsAttribute MoveSpeedMultiplier;

    
    UPROPERTY(EditAnywhere, Category=Stamina)
    FSoulsAttribute Stamina;

    UPROPERTY(EditAnywhere, Category=Stamina)
    FSoulsAttribute StaminaMax;

    
    UPROPERTY(EditAnywhere, Category=Stamina)
    FSoulsAttribute StaminaRegenRate;

    
    UPROPERTY(EditAnywhere, Category=Stamina)
    FSoulsAttribute StaminaRegenDelay;

    UPROPERTY(EditAnywhere, Category=Posture)
    FSoulsAttribute Posture;

    UPROPERTY(EditAnywhere, Category=Posture)
    FSoulsAttribute PostureMax;

    UPROPERTY(EditAnywhere, Category=Poise)
    FSoulsAttribute Poise;

    UPROPERTY(EditAnywhere, Category=Poise)
    FSoulsAttribute PoiseMax;

    UPROPERTY(EditAnywhere, Category=Defense)
    FSoulsAttribute Defense;

    virtual void PostAttributeChanged() override;
    virtual void InitializeAttributes() override;

    void ApplyMoveSpeed();

    USoulsPawnAttributeSet();
};


UCLASS()
class SOULLIKE_API USoulsPlayerAttributeSet : public USoulsPawnAttributeSet
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=PlayerResources)
    FSoulsAttribute FlaskCharges;

    UPROPERTY(EditAnywhere, Category=PlayerResources)
    FSoulsAttribute FocusPoints;

    USoulsPlayerAttributeSet();
};


UCLASS()
class SOULLIKE_API USoulsMonsterAttributeSet : public USoulsPawnAttributeSet
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category=MonsterRewards)
    FSoulsAttribute SoulsReward;

    USoulsMonsterAttributeSet();
};