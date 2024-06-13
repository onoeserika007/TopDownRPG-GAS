// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AuraGameplayTags.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(FAuraGameplayTags::GetInstance().Damage_Fire, Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches a bolt of fire, exploding on impact and dealing: </><Damage>%d</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"), Damage, Level, ManaCost, Cooldown);
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%d</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(NumProjectiles, Level), Damage, Level, ManaCost, Cooldown);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = GetDamageByDamageType(FAuraGameplayTags::GetInstance().Damage_Fire, Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%d</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(NumProjectiles, Level), Damage, Level, ManaCost, Cooldown);
}
