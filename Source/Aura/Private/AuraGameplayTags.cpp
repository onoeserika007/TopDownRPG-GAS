// Fill out your copyright notice in the Description page of Project Settings.


#include "AuraGameplayTags.h"
#include "GameplayTagsManager.h"

FAuraGameplayTags FAuraGameplayTags::GameplayTags {};

void FAuraGameplayTags::InitializeNativeGameplayTags()
{
	auto& Manager = UGameplayTagsManager::Get();
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

	/**
	 * Metas
	 */
	GameplayTags.Damage = Manager.AddNativeGameplayTag(
		FName("Damage"),
		FString("Damage.")
		);

	GameplayTags.Effects_HitReact = Manager.AddNativeGameplayTag(
		FName("Effects.HitReact"),
		FString("Tag granted when Hit Reacting.")
		);
	
}
