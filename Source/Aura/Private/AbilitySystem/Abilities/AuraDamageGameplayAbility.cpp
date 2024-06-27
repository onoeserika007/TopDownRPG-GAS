// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Chaos/Framework/Parallel.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* Victim)
{
	// You don't have to make effect context here.
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());

	const float ScaledDamage = DamageTable.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageTypeTag, ScaledDamage);
	// From this ASC apply to Target ASC
	const auto SourceASC = GetAbilitySystemComponentFromActorInfo();
	const auto TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data, TargetASC);
}

FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride,
	bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride, bool bOverridePitch, float PitchOverride) const
{
	FDamageEffectParams Params {};
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor); // NULL check has been down inside the function
	Params.BaseDamage = DamageTable.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageType = DamageTypeTag;
	Params.DebuffChance = DebuffChance;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffDuration = DebuffDuration;
	Params.DebuffFrequency = DebuffFrequency;
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackChance = KnockbackChance;
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;

	if (IsValid(TargetActor))
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		const FVector ToTarget = Rotation.Vector();
		Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
	}

	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
		Params.RadialDamageOrigin = InRadialDamageOrigin;
	}

	if (bOverrideKnockbackDirection)
	{
		FRotator KonockRotation = KnockbackDirectionOverride.Rotation();
		if (bOverridePitch)
		{
			KonockRotation.Pitch = PitchOverride;
		}
		Params.KnockbackForce = KonockRotation.Vector() * KnockbackForceMagnitude;
	}

	if (bOverrideDeathImpulse)
	{
		FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation();
		if (bOverridePitch)
		{
			DeathImpulseRotation.Pitch = PitchOverride;
		}
		Params.DeathImpulse = DeathImpulseRotation.Vector() * KnockbackForceMagnitude;
	}
	return Params;
}


float UAuraDamageGameplayAbility::GetDamageAtAbilityLevel() const
{
	return DamageTable.GetValueAtLevel(GetAbilityLevel());
}

FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages)
{
	if (TaggedMontages.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[Selection];
	}
	return {};
}

// float UAuraDamageGameplayAbility::GetDamageByDamageType(const FGameplayTag& DamageType, float InLevel)
// {
// 	// checkf(DamageTypes.Contains(DamageType), TEXT("GameplayAbility [%s] dose not contains DamageType [%s]"), *GetNameSafe(this), *DamageType.ToString());
// 	// return DamageTypes[DamageType].GetValueAtLevel(InLevel);
// 	return 0.f;
// }
