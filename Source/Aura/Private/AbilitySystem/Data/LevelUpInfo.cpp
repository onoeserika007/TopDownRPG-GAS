// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(const int32 XP) const
{
	// Convention: The first element of levels will be no level.
	int32 Level = 1;
	bool bSearching = true;
	while (bSearching)
	{
		// LevelUpInfos[1] = Level 1 Information
		if (LevelUpInfos.Num() - 1 <= Level)
		{
			return Level;
		}

		if (XP > LevelUpInfos[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}
	return Level;
}
