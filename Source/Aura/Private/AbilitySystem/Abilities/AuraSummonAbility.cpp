// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

#include "Kismet/KismetSystemLibrary.h"


TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const float DeltaSpread = SpawnSpread / NumMinions;

	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	UKismetSystemLibrary::DrawDebugArrow(
		GetAvatarActorFromActorInfo(),
		Location + LeftOfSpread * MinSpawnDistance,
		Location + LeftOfSpread * MaxSpawnDistance,
		4.f, 
		FLinearColor::Green,
		3.f);

	TArray<FVector> SpawnLocations;
	for (int i = 0; i < NumMinions; i++)
	{
		const FVector Division = LeftOfSpread.RotateAngleAxis(i * DeltaSpread, FVector::UpVector);
		UKismetSystemLibrary::DrawDebugArrow(
		GetAvatarActorFromActorInfo(),
		Location + Division * MinSpawnDistance,
		Location + Division * MaxSpawnDistance,
		4.f, 
		FLinearColor::Blue,
		3.f);
		
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread / 2.f + i * DeltaSpread, FVector::UpVector);
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		// Force on the ground
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(
			Hit,
			ChosenSpawnLocation + FVector(0.f, 0.f, 400.f),
			ChosenSpawnLocation - FVector(0.f, 0.f, 400.f),
			ECC_Visibility
			);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
			SpawnLocations.Add(ChosenSpawnLocation);
		}
		// UKismetSystemLibrary::DrawDebugSphere(
		// 	GetAvatarActorFromActorInfo(),
		// 	ChosenSpawnLocation,
		// 	18.f,
		// 	12,
		// 	FColor::Cyan,
		// 	3.f
		// 	);
	}

	const FVector RightOfSpread = Forward.RotateAngleAxis(SpawnSpread / 2.f, FVector::UpVector);
	UKismetSystemLibrary::DrawDebugArrow(
		GetAvatarActorFromActorInfo(),
		Location + RightOfSpread * MinSpawnDistance,
		Location + RightOfSpread * MaxSpawnDistance,
		4.f, 
		FLinearColor::Red,
		3.f);

	return SpawnLocations;
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
