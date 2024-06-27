// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	// const int32 Damage = GetDamageByDamageType(FAuraGameplayTags::GetInstance().Damage_Fire, Level);
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>ARCANE SHARDS</>\n\n"
			"<Default>Summon a shard of arcane enery, causing radial damage of: </><Damage>%.1f</> <Default>, have a chance to knockback</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"), Damage, Level, ManaCost, Cooldown);
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>ARCANE SHARDS</>\n\n"
			"<Default>Summon %d shard of arcane enery, causing radial damage of: </><Damage>%.1f</> <Default>, have a chance to knockback</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(MaxNumShards, Level), Damage, Level, ManaCost, Cooldown);
	}
}

FString UArcaneShards::GetNextLevelDescription(int32 Level)
{
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>ARCANE SHARDS</>\n\n"
			"<Default>Summon %d shard of arcane enery, causing radial damage of: </><Damage>%.1f</> <Default>, have a chance to knockback</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(MaxNumShards, Level), Damage, Level, ManaCost, Cooldown);
}
