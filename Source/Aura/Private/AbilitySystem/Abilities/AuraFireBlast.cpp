// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireBlast.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraFireBall.h"

FString UAuraFireBlast::GetDescription(int32 Level)
{
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
	"<Title>FIRE BLAST</>\n\n"
	"<Default>Launches %d fire balls in all directions, each coming back and exploding upon return, causing: </><Damage>%.1f</> <Default>radial fire damage, will have a chance to burn</>\n\n"
	"<Small>Level: </><Level>%d</>\n"
	"<Small>ManaCost: </><ManaCost>%.1f</>\n"
	"<Small>Cooldown: </><Cooldown>%.1f</>"), NumFireBalls, Damage, Level, ManaCost, Cooldown);
}

FString UAuraFireBlast::GetNextLevelDescription(int32 Level)
{
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
	"<Title>NEXT LEVEL</>\n\n"
	"<Default>Launches %d fire balls in all directions, each coming back and exploding upon return, causing: </><Damage>%.1f</> <Default>radial fire damage, will have a chance to burn</>\n\n"
	"<Small>Level: </><Level>%d</>\n"
	"<Small>ManaCost: </><ManaCost>%.1f</>\n"
	"<Small>Cooldown: </><Cooldown>%.1f</>"), NumFireBalls, Damage, Level, ManaCost, Cooldown);
}

TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
	TArray<AAuraFireBall*> RetFireBalls;
	const auto Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const auto Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const auto& Rotators = UAuraAbilitySystemLibrary::SpawnEvenlySpacedRotators(GetAvatarActorFromActorInfo(), Forward, FVector::UpVector, 360.f, NumFireBalls);

	for (const auto& Rot: Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rot.Quaternion());
		check(FireBallClass);
		AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
			FireBallClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());
		FireBall->FinishSpawning(SpawnTransform);

		RetFireBalls.Add(FireBall);
	}
	return RetFireBalls;
}
