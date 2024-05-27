// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AttributeInfo.h"

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

}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);

	check(AttributeInfo); // DataAsset

	for(auto& [Tag, AttrGetter]: AS->TagsToAttributes)
	{
		FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(Tag);
		Info.AttributeValue = AttrGetter().GetNumericValue(AS);
		AttributeInfoDelegate.Broadcast(Info);
	}
}
