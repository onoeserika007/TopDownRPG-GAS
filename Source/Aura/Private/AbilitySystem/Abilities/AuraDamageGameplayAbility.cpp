// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UAuraDamageGameplayAbility::CauseDamage(AActor* Victim)
{
	// You don't have to make effect context here.
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, GetAbilityLevel());
	for (auto& [Tag, DamageTable]: DamageTypes)
	{
		const float ScaledDamage = DamageTable.GetValueAtLevel(GetAbilityLevel());
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, Tag, ScaledDamage);
	}
	// From this ASC apply to Target ASC
	const auto SourceASC = GetAbilitySystemComponentFromActorInfo();
	const auto TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Victim);
	SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data, TargetASC);
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

float UAuraDamageGameplayAbility::GetDamageByDamageType(const FGameplayTag& DamageType, float InLevel)
{
	checkf(DamageTypes.Contains(DamageType), TEXT("GameplayAbility [%s] dose not contains DamageType [%s]"), *GetNameSafe(this), *DamageType.ToString());
	return DamageTypes[DamageType].GetValueAtLevel(InLevel);
}
