// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraFireBolt.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"

FString UAuraFireBolt::GetDescription(int32 Level)
{
	// const int32 Damage = GetDamageByDamageType(FAuraGameplayTags::GetInstance().Damage_Fire, Level);
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches a bolt of fire, exploding on impact and dealing: </><Damage>%.1f</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"), Damage, Level, ManaCost, Cooldown);
	}
	else
	{
		return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%.1f</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(NumProjectiles, Level), Damage, Level, ManaCost, Cooldown);
	}
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level)
{
	const float Damage = DamageTable.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			"<Title>FIRE BOLT</>\n\n"
			"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%.1f</> <Default>fire damage will have a chance to burn</>\n\n"
			"<Small>Level: </><Level>%d</>\n"
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			"<Small>Cooldown: </><Cooldown>%.1f</>"),
			FMath::Min(NumProjectiles, Level), Damage, Level, ManaCost, Cooldown);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& TargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch,
	float PitchOverride, AActor* HomingTarget)
{
	if (!GetAvatarActorFromActorInfo() || !GetAvatarActorFromActorInfo()->HasAuthority()) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
			GetAvatarActorFromActorInfo(),
			SocketTag);
	FRotator ToTargetRotation = (TargetLocation - SocketLocation).Rotation();
	// ToTargetRotation.Pitch = 0.f; This may cause not hit on server when use dedicated server mode.
	if (bOverridePitch)
	{
		ToTargetRotation.Pitch = PitchOverride;
	}

	// UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + ToTargetRotation.Vector() * 100.f, 2, FLinearColor::Yellow, 120.f);

	const int32 NumOfProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
	const FVector Forward = ToTargetRotation.Vector();
	const auto& Directions = UAuraAbilitySystemLibrary::SpawnEvenlyRotatedVectors(GetAvatarActorFromActorInfo(), Forward, FVector::UpVector, ProjectileSpread, NumOfProjectiles);

	for (const auto& Direction: Directions)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Direction.Rotation().Quaternion());
		AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
			ProjectileClass,
			SpawnTransform,
			GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		
		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(TargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
		Projectile->FinishSpawning(SpawnTransform);
	}

}
