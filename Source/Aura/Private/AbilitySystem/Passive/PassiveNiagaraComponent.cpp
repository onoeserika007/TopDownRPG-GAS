// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAuraAbilitySystemComponent* AuraAsc = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		AuraAsc->ActivatePassiveEffectDelegate.AddUObject(this, &ThisClass::OnPassiveActivate);
		ActivateIfEquipped(AuraAsc); // In case the Passive Ability has been activated when loaded from disk saved data.
	}
	else if (TScriptInterface<ICombatInterface> CombatInterface = GetOwner())
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* InAsc)
		{
			if (UAuraAbilitySystemComponent* AuraAsc = Cast<UAuraAbilitySystemComponent>(InAsc))
			{
				AuraAsc->ActivatePassiveEffectDelegate.AddUObject(this, &ThisClass::OnPassiveActivate);
				ActivateIfEquipped(AuraAsc); // In case the Passive Ability has been activated when loaded from disk saved data.
			}
		});
	}
}

void UPassiveNiagaraComponent::ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC)
{
	// In case the Passive Ability has been activated when loaded from disk saved data.
	const bool bStartupAbilitiesActivated = AuraASC->bStartupAbilitiesGiven;
	if (bStartupAbilitiesActivated)
	{
		if (AuraASC->GetStatusFromAbilityTag(PassiveSpellTag).MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped))
		{
			Activate();
		}
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
}
