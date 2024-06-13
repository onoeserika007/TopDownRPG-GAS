// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AuraGameplayTags.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// The blueprint version of print string to screen
	// UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (C++)"), true, true, FLinearColor::Yellow, 3);
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& TargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
	if (GetAvatarActorFromActorInfo()->HasAuthority())
	{
		if (GetAvatarActorFromActorInfo()) // Who owns the ability
		{
			const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
				GetAvatarActorFromActorInfo(),
				SocketTag);
			FRotator ToTargetRotation = (TargetLocation - SocketLocation).Rotation();
			// ToTargetRotation.Pitch = 0.f; This may cause not hit on server when use dedicated server mode.
			if (bOverridePitch)
			{
				ToTargetRotation.Pitch = PitchOverride;
			}
			
			FTransform SpawnTransform;
			SpawnTransform.SetLocation(SocketLocation);
			SpawnTransform.SetRotation(ToTargetRotation.Quaternion());
 
			/**
			 *	SpawnActor: 无法在生成之后和初始化之前配置Actor。任何配置必须在PostInitializeComponents之后进行。
			 *	SpawnActorDeferred: 允许在生成之后和初始化之前进行配置，使得你可以在Actor的生命周期早期设置属性。
			 */
			check(ProjectileClass);
			AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
				ProjectileClass,
				SpawnTransform,
				GetOwningActorFromActorInfo(),
				Cast<APawn>(GetOwningActorFromActorInfo()),
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
				
			// Deferred PostInitializeComponents
			// TODO: Give the Projectile a Gameplay Effect Spec for causing Damage.

			const auto SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());
			FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
			// Instigator(Owner) and EffectCauser(Avatar) are automatically set when MakeEffectContext.
			EffectContextHandle.SetAbility(this); // Manually
			EffectContextHandle.AddSourceObject(Projectile); // Manually
			TArray<TWeakObjectPtr<AActor>> Actors; // What the hell is the point of this?
			Actors.Add(Projectile);
			EffectContextHandle.AddActors(Actors);
			FHitResult HitResult;
			HitResult.Location = TargetLocation;
			EffectContextHandle.AddHitResult(HitResult);
			
			const auto SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, GetAbilityLevel(), EffectContextHandle);
			if (Projectile)
			{
				const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();

				// Four Damage types, Execution can capture SetByCalled value by these DamageType Tags.
				for (auto& [Tag, DamageTable]: DamageTypes)
				{
					const float ScaledDamage = DamageTable.GetValueAtLevel(GetAbilityLevel());
					UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, Tag, ScaledDamage);
				}

				// When the projectile collide, only need to apply the GE;
				Projectile->DamageEffectSpecHandle = SpecHandle;
				Projectile->FinishSpawning(SpawnTransform);
			}
			
		}
	}
}
