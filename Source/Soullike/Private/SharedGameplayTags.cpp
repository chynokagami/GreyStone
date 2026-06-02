#include "SharedGameplayTags.h"

namespace SharedGameplayTags
{
    //  Status 
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsRolling, "Status.IsRolling", "Character is currently in rolling animation");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsBlocking, "Status.IsBlocking", "Character is currently blocking with shield/weapon");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsAttacking, "Status.IsAttacking", "Character is in middle of attack action");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsStunned, "Status.IsStunned", "Character is in hit-stun, cannot act");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsDead, "Status.IsDead", "Character is dead");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsLockedOn, "Status.IsLockedOn", "Player has a target locked on");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IFrame, "Status.IFrame", "Invincibility frames active (no damage taken)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_IsExhausted, "Status.IsExhausted", "Stamina depleted - reduced damage and slow recovery");

    //  Action 
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_LightAttack, "Action.LightAttack", "Standard fast attack");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_HeavyAttack, "Action.HeavyAttack", "Charged heavy attack");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Roll, "Action.Roll", "Dodge roll with IFrames");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Block, "Action.Block", "Hold to block incoming damage");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Parry, "Action.Parry", "Frame-perfect deflect");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_UseFlask, "Action.UseFlask", "Drink healing flask");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_Sprint, "Action.Sprint", "Sprint with stamina drain");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Action_LockOn, "Action.LockOn", "Player locks camera onto target");

    //  Team 
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Team_Player, "Team.Player", "Allied with player");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Team_Enemy, "Team.Enemy", "Hostile to player");

    //  Attribute 
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Health, "Attribute.Health", "Current health");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_HealthMax, "Attribute.HealthMax", "Max health cap");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Stamina, "Attribute.Stamina", "Current stamina (used as activation cost key)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_StaminaMax, "Attribute.StaminaMax", "Max stamina cap");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_StaminaRegenRate, "Attribute.StaminaRegenRate", "Stamina recovery per second");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_StaminaRegenDelay, "Attribute.StaminaRegenDelay", "Delay before stamina starts regenerating after consumption");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Posture, "Attribute.Posture", "Posture meter (build-up, breaks at max)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_PostureMax, "Attribute.PostureMax", "Max posture cap");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Poise, "Attribute.Poise", "Poise (resists hit-stun)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_PoiseMax, "Attribute.PoiseMax", "Max poise cap");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MoveSpeed, "Attribute.MoveSpeed", "Walking speed base value");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_MoveSpeedMultiplier, "Attribute.MoveSpeedMultiplier", "Move speed multiplier (sprint, slow effects)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_Defense, "Attribute.Defense", "Damage reduction value");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_FlaskCharges, "Attribute.FlaskCharges", "Healing flask remaining charges");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_FocusPoints, "Attribute.FocusPoints", "Spell casting focus points");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Attribute_SoulsReward, "Attribute.SoulsReward", "Souls dropped on enemy death");

    //  Damage 
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Physical, "Damage.Physical", "Physical damage (root)");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Slash, "Damage.Slash", "Slashing physical");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Strike, "Damage.Strike", "Blunt strike physical");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Thrust, "Damage.Thrust", "Piercing thrust");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Fire, "Damage.Fire", "Fire elemental");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Lightning, "Damage.Lightning", "Lightning elemental");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(Damage_Holy, "Damage.Holy", "Holy elemental");
}