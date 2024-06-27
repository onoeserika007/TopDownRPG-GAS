// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Kismet/KismetSystemLibrary.h"


TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const float DeltaSpread = SpawnSpread / NumMinions;
	return UAuraAbilitySystemLibrary::SpawnEvenlySpacedLocations(GetAvatarActorFromActorInfo(), Forward, Location, SpawnSpread, NumMinions, MinSpawnDistance, MaxSpawnDistance);
}

TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	if (MinionsClasses.Num() > 0)
	{
		int32 Selection = FMath::RandRange(0, MinionsClasses.Num() - 1);
		return MinionsClasses[Selection];
	}
	return {};
}
