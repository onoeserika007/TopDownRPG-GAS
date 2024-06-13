// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/**
 * AuraGameplayTags
 * 
 * Singleton containing native Gameplay Tags
 */

struct  FAuraGameplayTags
{
public:
 static const FAuraGameplayTags& GetInstance() { return GameplayTags;}
 static void InitializeNativeGameplayTags();
 /**
  * Primary Attributes
  */
 FGameplayTag Attributes_Primary_Strength;
 FGameplayTag Attributes_Primary_Intelligence;
 FGameplayTag Attributes_Primary_Resilience;
 FGameplayTag Attributes_Primary_Vigor;

 /**
  * Secondary Attributes
  */
 FGameplayTag Attributes_Secondary_Armor;
 FGameplayTag Attributes_Secondary_ArmorPenetration;
 FGameplayTag Attributes_Secondary_BlockChance;
 FGameplayTag Attributes_Secondary_CriticalHitChance;
 FGameplayTag Attributes_Secondary_CriticalHitDamage;
 FGameplayTag Attributes_Secondary_CriticalHitResistance;
 FGameplayTag Attributes_Secondary_HealthRegeneration;
 FGameplayTag Attributes_Secondary_ManaRegeneration;
 FGameplayTag Attributes_Secondary_MaxHealth;
 FGameplayTag Attributes_Secondary_MaxMana;

 /**
  * Input Action Tags
  */
 FGameplayTag InputTag_LMB;
 FGameplayTag InputTag_RMB;
 FGameplayTag InputTag_1;
 FGameplayTag InputTag_2;
 FGameplayTag InputTag_3;
 FGameplayTag InputTag_4;

 FGameplayTag InputTag_Passive_1;
 FGameplayTag InputTag_Passive_2;

 /**
  * Metas
  */
 FGameplayTag Damage; // This tag used to set damage by caller, now is substituted by Damage.Fire etc.
 // UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude
 FGameplayTag Attributes_Meta_IncomingXP;
 /**
  * Damage Types
  */
 FGameplayTag Damage_Fire;
 FGameplayTag Damage_Lightning;
 FGameplayTag Damage_Arcane;
 FGameplayTag Damage_Physical;

 /**
  * Resistance
  */
 FGameplayTag Attributes_Resistance_Fire;
 FGameplayTag Attributes_Resistance_Lightning;
 FGameplayTag Attributes_Resistance_Arcane;
 FGameplayTag Attributes_Resistance_Physical;

 TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;

 /**
  * Effects
  */
 FGameplayTag Effects_HitReact;
 
 /**
  * Abilities
  */
 FGameplayTag Abilities_None;
 FGameplayTag Abilities_Attack;
 FGameplayTag Abilities_Summon;
 FGameplayTag Abilities_HitReact;

 FGameplayTag Abilities_Status_Locked;
 FGameplayTag Abilities_Status_Eligible;
 FGameplayTag Abilities_Status_Unlocked;
 FGameplayTag Abilities_Status_Equipped;

 FGameplayTag Abilities_Type_Offensive;
 FGameplayTag Abilities_Type_Passive;
 FGameplayTag Abilities_Type_None;
 
 FGameplayTag Abilities_Fire_FireBolt;
 FGameplayTag Abilities_Lightning_Electrocute;
 
 /**
  * Ability Cooldowns
  */
 FGameplayTag Cooldown_Fire_FireBolt;
 FGameplayTag Cooldown_Lightning_Electrocute;
 

 /**
  * Combat Sockets
  */
 FGameplayTag CombatSocket_Weapon;
 FGameplayTag CombatSocket_LeftHand;
 FGameplayTag CombatSocket_RightHand;
 FGameplayTag CombatSocket_Tail;

 /**
  * Montage
  */
 FGameplayTag Montage_Attack_1;
 FGameplayTag Montage_Attack_2;
 FGameplayTag Montage_Attack_3;
 FGameplayTag Montage_Attack_4;
protected:
private:
 static FAuraGameplayTags GameplayTags;
};
