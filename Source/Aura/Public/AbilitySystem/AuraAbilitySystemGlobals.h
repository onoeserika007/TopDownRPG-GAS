#pragma once
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * Add to Config/DefaultGame.ini:
 * [/Script/GameplayAbilities.AbilitySystemGlobals]
 * +AbilitySystemGlobalsClassName="/Script/Aura.AuraAbilitySystemGlobals"
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals: public UAbilitySystemGlobals
{
	GENERATED_BODY()

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};