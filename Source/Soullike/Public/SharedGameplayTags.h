#pragma once

#include "NativeGameplayTags.h"


namespace SharedGameplayTags
{
    //Status
    //  Tag ActionComp->ActiveGameplayTags
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsRolling);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsBlocking);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsAttacking);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsStunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsDead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsLockedOn);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IFrame);       // 
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Status_IsExhausted);  // 

    // Action
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_LightAttack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_HeavyAttack);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Roll);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Block);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Parry);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_UseFlask);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Sprint);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_LockOn);

    // Team
    // Used by weapon hit detection to skip friendly fire.
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Player);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Team_Enemy);

    //Attribute
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Health);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_HealthMax);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Stamina);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_StaminaMax);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_StaminaRegenRate);   
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_StaminaRegenDelay);  
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Posture);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_PostureMax);         
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Poise);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_PoiseMax);           
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MoveSpeed);          
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_MoveSpeedMultiplier); 
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_Defense);            
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_FlaskCharges);       
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_FocusPoints);        
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attribute_SoulsReward);

    //Damage
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Physical);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Slash);   
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Strike);   
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Thrust);  
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Fire);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Lightning);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Damage_Holy);
}
