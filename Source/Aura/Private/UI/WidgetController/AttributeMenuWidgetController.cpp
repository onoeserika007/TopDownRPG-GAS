// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	// This is done in blueprint
	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	check(AttributeInfo); // DataAsset
	
	for(auto& [Tag, AttrGetter]: AS->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttrGetter()).AddLambda(
			// lifecycle is considered, don't use ref capture
			[this, Tag, AttrGetter, AS](const FOnAttributeChangeData& Data)
			{
				FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Tag);
				Info.AttributeValue = AttrGetter().GetNumericValue(AS);
				AttributeInfoDelegate.Broadcast(Info);
			}
		);
	}

	AAuraPlayerState* AuraAS = GetAuraPlayerState();
	AuraAS->OnAttributePointsChangedDelegate.AddLambda([this](int32 InPoints)
	{
		AttributePointsChangedDelegate.Broadcast(InPoints);
	});

	AuraAS->OnSpellPointsChangedDelegate.AddLambda([this](int32 InPoints)
	{
		SpellPointsChangedDelegate.Broadcast(InPoints);
	});

}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	check(AttributeInfo); // DataAsset

	// Since Attribute Rows bind to delegate AttributeInfoDelegate on their event construct, so these broadcast must have
	// happened after binding.
	for(auto& [Tag, AttrGetter]: AS->TagsToAttributes)
	{
		FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Tag);
		Info.AttributeValue = AttrGetter().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}

	const AAuraPlayerState* AuraPS = GetAuraPlayerState();
	AttributePointsChangedDelegate.Broadcast(AuraPS->GetAttributePoints());
	SpellPointsChangedDelegate.Broadcast(AuraPS->GetSpellPoints());
}

TArray<FGameplayTag> UAttributeMenuWidgetController::GetAttributeTags() const
{
	TArray<FGameplayTag> AttributeTags;
	if (UAuraAttributeSet* AuraAS = GetAuraAttributeSet())
	{
		for (auto& [Tag, _]: AuraAS->TagsToAttributes)
		{
			AttributeTags.AddUnique(Tag);
		}
	}
	return AttributeTags;
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	AuraASC->UpgradeAttribute(AttributeTag);
}
