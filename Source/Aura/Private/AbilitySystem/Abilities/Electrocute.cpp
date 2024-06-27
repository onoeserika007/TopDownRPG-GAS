// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Electrocute.h"

FString UElectrocute::GetDescription(int32 Level)
{
	// const int32 Damage = GetDamageByDamageType(FAuraGameplayTags::GetInstance().Damage_Fire, Level);
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Default>Emits a beam of lighting, connecting with the target, repeatedly causing: </><Damage>%.1f</> <Default>lightning damage, have a chance to stun</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"), Damage, Level, ManaCost, Cooldown);
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>ELECTROCUTE</>\n\n"
			"<Default>Emits %d beam of lighting, connecting with the target, repeatedly causing: </><Damage>%.1f</> <Default>lightning damage, have a chance to stun</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(MaxNumShockTargets, Level), Damage, Level, ManaCost, Cooldown);
	}
}

FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>NEXT LEVEL</>\n\n"
			"<Default>Emits %d beam of lighting, connecting with the target, repeatedly causing: </><Damage>%.1f</> <Default>lightning damage,  have a chance to stun</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(MaxNumShockTargets, Level), Damage, Level, ManaCost, Cooldown);
}
