// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AuraWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerController.h"
#include "Player/AuraPlayerState.h"

AAuraPlayerController* UAuraWidgetController::GetAuraPlayerController() const
{
	if (!AuraPlayerController)
	{
		AuraPlayerController = CastChecked<AAuraPlayerController>(PlayerController);
	}
	return AuraPlayerController;
}

AAuraPlayerState* UAuraWidgetController::GetAuraPlayerState() const
{
	if (!AuraPlayerState)
	{
		AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	}
	return AuraPlayerState;
}

UAuraAbilitySystemComponent* UAuraWidgetController::GetAuraAbilitySystemComponent() const
{
	if (!AuraAbilitySystemComponent)
	{
		AuraAbilitySystemComponent = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	}
	return AuraAbilitySystemComponent;
}

UAuraAttributeSet* UAuraWidgetController::GetAuraAttributeSet() const
{
	if (!AuraAttributeSet)
	{
		AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
	}
	return AuraAttributeSet;
}

void UAuraWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& Params)
{
	PlayerController = Params.PlayerController;
	PlayerState = Params.PlayerState;
	AbilitySystemComponent = Params.AbilitySystemComponent;
	AttributeSet = Params.AttributeSet;
}

void UAuraWidgetController::BroadcastInitialValues()
{
}

void UAuraWidgetController::BindCallbacksToDependencies()
{
}

void UAuraWidgetController::BroadcastAbilityInfo() const
{
	OnInitializeStartupAbilities(GetAuraAbilitySystemComponent());
}

void UAuraWidgetController::OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraASC) const
{
	// TODO: Get information about all given abilities, look up their Ability Info, and broadcast it to widgets.
	if (AuraASC && AuraASC->bStartupAbilitiesGiven)
	{
		FForEachAbility BroadcastDelegate;
		BroadcastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
		{
			// TODO: need a way to figure out the ability tag for a given ability spec.
			FAuraAbilityInfo AuraAbilityInfo =  AbilityInfo->FindAbilityInfoForTag(UAuraAbilitySystemComponent::GetAbilityTagFromSpec(AbilitySpec));
			AuraAbilityInfo.InputTag = UAuraAbilitySystemComponent::GetInputTagFromSpec(AbilitySpec);
			AuraAbilityInfo.StatusTag = UAuraAbilitySystemComponent::GetStatusTagFromSpec(AbilitySpec);
			AbilityInfoDelegate.Broadcast(AuraAbilityInfo);
		});
		
		// Instead of binding delegates on other components to retrieve infos, we bind delegate in our own scope,
		// and send it to other components to get infos.
		// So why not just retrieve the Activatable Abilities explicitly?
		// Maybe it's for the lock.
		AuraASC->ForEachAbility(BroadcastDelegate);
	}
}
