// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	auto& Manager = UGameplayTagsManager::Get();
	FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
	/**
	 * Primary
	 */
	GameplayTags.Attributes_Primary_Strength = Manager.AddNativeGameplayTag(
		FName("Attributes.Primary.Strength"),
		FString("Increase physical damage.")
		);

	GameplayTags.Attributes_Primary_Intelligence = Manager.AddNativeGameplayTag(
		FName("Attributes.Primary.Intelligence"),
		FString("Increase magical damage.")
		);

	GameplayTags.Attributes_Primary_Resilience = Manager.AddNativeGameplayTag(
		FName("Attributes.Primary.Resilience"),
		FString("Increase Armor and Armo Penetration.")
		);

	GameplayTags.Attributes_Primary_Vigor = Manager.AddNativeGameplayTag(
		FName("Attributes.Primary.Vigor"),
		FString("Increase Health.")
		);

	/**
	 * Secondary
	 */
	GameplayTags.Attributes_Secondary_Armor = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.Armor"),
		FString("Reduces damage taken, improves Block Chance.")
		);

	GameplayTags.Attributes_Secondary_ArmorPenetration = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.ArmorPenetration"),
		FString("Ignores percentage of enemy Armor, increases Critical Hit Chance.")
		);

	GameplayTags.Attributes_Secondary_BlockChance = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.BlockChance"),
		FString("Chance to cut incoming damage in half.")
		);

	GameplayTags.Attributes_Secondary_CriticalHitChance = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitChance"),
		FString("Chance to double plus critical hit bonus.")
		);

	GameplayTags.Attributes_Secondary_CriticalHitDamage = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitDamage"),
		FString("Bonus damage added when a critical hit is scored.")
		);

	GameplayTags.Attributes_Secondary_CriticalHitResistance = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.CriticalHitResistance"),
		FString("Reduces Critical Hit Chance of attacking enemies.")
		);
	
	GameplayTags.Attributes_Secondary_HealthRegeneration = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.HealthRegeneration"),
		FString("Amount of health regenerated every second.")
		);

	GameplayTags.Attributes_Secondary_ManaRegeneration = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.ManaRegeneration"),
		FString("Amount of mana regenerated every second.")
		);

	GameplayTags.Attributes_Secondary_MaxHealth = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxHealth"),
		FString("Maximum amount of Health obtainable.")
		);

	GameplayTags.Attributes_Secondary_MaxMana = Manager.AddNativeGameplayTag(
		FName("Attributes.Secondary.MaxMana"),
		FString("Maximum amount of Mana obtainable.")
		);
	
	/**
	 * Inputs
	 */
	GameplayTags.InputTag_LMB = Manager.AddNativeGameplayTag(
		FName("InputTag.LMB"),
		FString("Input Tag for Left Mouse Button.")
		);

	GameplayTags.InputTag_RMB = Manager.AddNativeGameplayTag(
		FName("InputTag.RMB"),
		FString("Input Tag for Right Mouse Button.")
		);

	GameplayTags.InputTag_1 = Manager.AddNativeGameplayTag(
		FName("InputTag.1"),
		FString("Input Tag for 1 key.")
		);

	GameplayTags.InputTag_2 = Manager.AddNativeGameplayTag(
		FName("InputTag.2"),
		FString("Input Tag for 2 key.")
		);

	GameplayTags.InputTag_3 = Manager.AddNativeGameplayTag(
		FName("InputTag.3"),
		FString("Input Tag for 3 key.")
		);

	GameplayTags.InputTag_4 = Manager.AddNativeGameplayTag(
		FName("InputTag.4"),
		FString("Input Tag for 4 key.")
		);

	GameplayTags.InputTag_Passive_1 = Manager.AddNativeGameplayTag(
		FName("InputTag.Passive.1"),
		FString("Input Tag for Passive Ability 1.")
		);

	GameplayTags.InputTag_Passive_2= Manager.AddNativeGameplayTag(
		FName("InputTag.Passive.2"),
		FString("Input Tag for Passive Ability 2.")
		);

	/**
	 * Player Inputs
	 */
	GameplayTags.Player_Block_CursorTrace= Manager.AddNativeGameplayTag(
		FName("Player.Block.CursorTrace"),
		FString("Block tracing under cursor.")
		);
	
	GameplayTags.Player_Block_InputHeld= Manager.AddNativeGameplayTag(
		FName("Player.Block.InputHeld"),
		FString("Block Input Held callback for input.")
		);
	
	GameplayTags.Player_Block_InputHeld= Manager.AddNativeGameplayTag(
		FName("Player.Block.InputPressed"),
		FString("Block Input Pressed callback for input.")
		);

	GameplayTags.Player_Block_InputReleased= Manager.AddNativeGameplayTag(
		FName("Player.Block.InputReleased"),
		FString("Block Input Released callback for input.")
		);

	/**
	 * Metas
	 */
	GameplayTags.Damage = Manager.AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage.")
		);

	GameplayTags.Attributes_Meta_IncomingXP = Manager.AddNativeGameplayTag(
		FName("Attributes.Meta.IncomingXP"),
		FString("Incoming XP Meta Attribute.")
		);

	/**
	 * Damage Types
	 */
	GameplayTags.Damage_Fire = Manager.AddNativeGameplayTag(
	FName("Damage.Fire"),
	FString("Fire Damage.")
	);

	GameplayTags.Damage_Lightning = Manager.AddNativeGameplayTag(
	FName("Damage.Lightning"),
	FString("Lightning Damage.")
	);
	
	GameplayTags.Damage_Arcane = Manager.AddNativeGameplayTag(
	FName("Damage.Arcane"),
	FString("Arcane Damage.")
	);

	GameplayTags.Damage_Physical = Manager.AddNativeGameplayTag(
	FName("Damage.Physical"),
	FString("Physical Damage.")
	);

	/**
	 * Resistances
	 */
	GameplayTags.Attributes_Resistance_Fire = Manager.AddNativeGameplayTag(
	FName("Attributes.Resistance.Fire"),
	FString("Resistance to Fire Damage.")
	);

	GameplayTags.Attributes_Resistance_Lightning = Manager.AddNativeGameplayTag(
	FName("Attributes.Resistance.Lightning"),
	FString("Resistance to Lightning Damage.")
	);

	GameplayTags.Attributes_Resistance_Arcane = Manager.AddNativeGameplayTag(
	FName("Attributes.Resistance.Arcane"),
	FString("Resistance to Arcane Damage.")
	);

	GameplayTags.Attributes_Resistance_Physical = Manager.AddNativeGameplayTag(
	FName("Attributes.Resistance.Physical"),
	FString("Resistance to Physical Damage.")
	);
	
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Lightning, GameplayTags.Attributes_Resistance_Lightning);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Arcane, GameplayTags.Attributes_Resistance_Arcane);
	GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Physical, GameplayTags.Attributes_Resistance_Physical);

	/**
	 * Debuffs
	 */
	GameplayTags.Debuff_Burn = Manager.AddNativeGameplayTag(
	FName("Debuff.Burn"),
	FString("Debuff for fire damage.")
	);

	GameplayTags.Debuff_Stun = Manager.AddNativeGameplayTag(
	FName("Debuff.Stun"),
	FString("Debuff for lightning damage.")
	);

	GameplayTags.Debuff_Arcane = Manager.AddNativeGameplayTag(
	FName("Debuff.Arcane"),
	FString("Debuff for Arcane damage.")
	);

	GameplayTags.Debuff_Physical = Manager.AddNativeGameplayTag(
	FName("Debuff.Physical"),
	FString("Debuff for Physical damage.")
	);

	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Fire, GameplayTags.Debuff_Burn);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Lightning, GameplayTags.Debuff_Stun);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Arcane, GameplayTags.Debuff_Arcane);
	GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Physical, GameplayTags.Debuff_Physical);

	GameplayTags.Debuff_Chance = Manager.AddNativeGameplayTag(
	FName("Debuff.Chance"),
	FString("Debuff Chance.")
	);

	GameplayTags.Debuff_Damage = Manager.AddNativeGameplayTag(
	FName("Debuff.Damage"),
	FString("Debuff Damage.")
	);

	GameplayTags.Debuff_Duration = Manager.AddNativeGameplayTag(
	FName("Debuff.Duration"),
	FString("Debuff Duration.")
	);

	GameplayTags.Debuff_Frequency = Manager.AddNativeGameplayTag(
	FName("Debuff.Frequency"),
	FString("Debuff Frequency.")
	);
	
	/**
	 * Effects
	 */
	GameplayTags.Effects_HitReact = Manager.AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString("Tag granted when Hit Reacting.")
		);

	/**
	 * Abilities
	 */
	GameplayTags.Abilities_None = Manager.AddNativeGameplayTag(
		FName("Abilities.None"),
		FString("None Ability - like the nullptr for Ability Tags.")
		);
	
	GameplayTags.Abilities_Attack = Manager.AddNativeGameplayTag(
		FName("Abilities.Attack"),
		FString("Attack Ability Tag.")
		);

	GameplayTags.Abilities_Summon = Manager.AddNativeGameplayTag(
		FName("Abilities.Summon"),
		FString("Summon Ability Tag.")
		);

	GameplayTags.Abilities_HitReact = Manager.AddNativeGameplayTag(
		FName("Abilities.HitReact"),
		FString("HitReact Ability Tag.")
		);

	GameplayTags.Abilities_Status_Eligible = Manager.AddNativeGameplayTag(
		FName("Abilities.Status.Eligible"),
		FString("Eligible Status.")
		);
	GameplayTags.Abilities_Status_Equipped = Manager.AddNativeGameplayTag(
		FName("Abilities.Status.Equipped"),
		FString("Equipped Status.")
		);
	GameplayTags.Abilities_Status_Locked = Manager.AddNativeGameplayTag(
		FName("Abilities.Status.Locked"),
		FString("Locked Status.")
		);
	GameplayTags.Abilities_Status_Unlocked = Manager.AddNativeGameplayTag(
		FName("Abilities.Status.Unlocked"),
		FString("Unlocked Status.")
		);

	/**
	 * Ability Types
	 */
	GameplayTags.Abilities_Type_None = Manager.AddNativeGameplayTag(
		FName("Abilities.Type.None"),
		FString("Type None.")
		);
	GameplayTags.Abilities_Type_Offensive = Manager.AddNativeGameplayTag(
		FName("Abilities.Type.Offensive"),
		FString("Type Offensive.")
		);
	GameplayTags.Abilities_Type_Passive = Manager.AddNativeGameplayTag(
		FName("Abilities.Type.Passive"),
		FString("Type Passive.")
		);
	
	/**
	 * Offensive Abilities
	 */
	GameplayTags.Abilities_Fire_FireBolt = Manager.AddNativeGameplayTag(
		FName("Abilities.Fire.FireBolt"),
		FString("FireBolt Ability Tag.")
		);

	GameplayTags.Abilities_Fire_FireBlast = Manager.AddNativeGameplayTag(
		FName("Abilities.Fire.FireBlast"),
		FString("FireBlast Ability Tag.")
		);

	GameplayTags.Abilities_Lightning_Electrocute = Manager.AddNativeGameplayTag(
		FName("Abilities.Lightning.Electrocute"),
		FString("Electrocute Ability Tag.")
		);

	GameplayTags.Abilities_Arcane_ArcaneShards = Manager.AddNativeGameplayTag(
		FName("Abilities.Arcane.ArcaneShards"),
		FString("ArcaneShards Ability Tag.")
		);
	
	/**
	 * Passive Abilities
	 */
	GameplayTags.Abilities_Passive_HaloOfProtection = Manager.AddNativeGameplayTag(
		FName("Abilities.Passive.HaloOfProtection"),
		FString("Halo Of Protection.")
		);

	GameplayTags.Abilities_Passive_LifeSiphon = Manager.AddNativeGameplayTag(
		FName("Abilities.Passive.LifeSiphon"),
		FString("Life Siphon.")
		);
	
	GameplayTags.Abilities_Passive_ManaSiphon = Manager.AddNativeGameplayTag(
		FName("Abilities.Passive.ManaSiphon"),
		FString("Mana Siphon.")
		);
	
	/**
	 * Abilities Cooldowns
	 */
	GameplayTags.Cooldown_Fire_FireBolt = Manager.AddNativeGameplayTag(
		FName("Cooldown.Fire.FireBolt"),
		FString("FireBolt Cooldown Tag.")
		);

	GameplayTags.Cooldown_Fire_FireBlast = Manager.AddNativeGameplayTag(
		FName("Cooldown.Fire.FireBlast"),
		FString("FireBlast Cooldown Tag.")
		);

	GameplayTags.Cooldown_Lightning_Electrocute = Manager.AddNativeGameplayTag(
		FName("Cooldown.Lightning.Electrocute"),
		FString("Electrocute Cooldown Tag.")
		);

	GameplayTags.Cooldown_Arcane_ArcaneShards = Manager.AddNativeGameplayTag(
		FName("Cooldown.Arcane.ArcaneShards"),
		FString("ArcaneShards Cooldown Tag.")
		);

	/**
	 * Combat Sockets
	 */
	GameplayTags.CombatSocket_Weapon = Manager.AddNativeGameplayTag(
		FName("CombatSocket.Weapon"),
		FString("Weapon.")
		);

	GameplayTags.CombatSocket_LeftHand = Manager.AddNativeGameplayTag(
		FName("CombatSocket.LeftHand"),
		FString("Left Hand.")
		);

	GameplayTags.CombatSocket_RightHand = Manager.AddNativeGameplayTag(
		FName("CombatSocket.RightHand"),
		FString("Right Hand.")
		);
	
	GameplayTags.CombatSocket_Tail = Manager.AddNativeGameplayTag(
		FName("CombatSocket.Tail"),
		FString("Tail.")
		);

	/**
	 * Montage
	 */
	GameplayTags.Montage_Attack_1 = Manager.AddNativeGameplayTag(
		FName("Montage.Attack.1"),
		FString("Attack 1.")
		);

	GameplayTags.Montage_Attack_2 = Manager.AddNativeGameplayTag(
		FName("Montage.Attack.2"),
		FString("Attack 2.")
		);

	GameplayTags.Montage_Attack_3 = Manager.AddNativeGameplayTag(
		FName("Montage.Attack.3"),
		FString("Attack 3.")
		);

	GameplayTags.Montage_Attack_4 = Manager.AddNativeGameplayTag(
		FName("Montage.Attack.4"),
		FString("Attack 4.")
		);
	
	/**
	 * GameplayCue
	 */
	GameplayTags.GameplayCue_FireBlast = Manager.AddNativeGameplayTag(
		FName("GameplayCue.FireBlast"),
		FString("GameplayCue FireBlast")
		);
}
