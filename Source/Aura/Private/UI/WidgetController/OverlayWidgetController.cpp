// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"

#include "AuraGameplayTags.h"
#include "NiagaraValidationRule.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	const UAuraAttributeSet* AuraAS = GetAuraAttributeSet();
	OnHealthChanged.Broadcast(AuraAS->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAS->GetMana());
	OnMaxManaChanged.Broadcast(AuraAS->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();

	AAuraPlayerState* AuraPS = GetAuraPlayerState();
	AuraPS->OnXPChangedDelegate.AddUObject(this, &ThisClass::OnXPChanged);
	AuraPS->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bShowMessage)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bShowMessage);
		});
	
	UAuraAbilitySystemComponent* AuraASC = GetAuraAbilitySystemComponent();
	const UAuraAttributeSet* AuraAS = GetAuraAttributeSet();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnManaChanged.Broadcast(Data.NewValue);
			}
		);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxManaAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxManaChanged.Broadcast(Data.NewValue);
			}
		);

	// Where we broadcast the tag messages when pickup.
	AuraASC->EffectAssetTagsDelegate.AddLambda(
		[this](const FGameplayTagContainer& AssetTags)
		{
			for (const auto& Tag: AssetTags)
			{
				// TODO: Broadcast the tag to the widget controller.
				const FString Msg = FString::Printf(TEXT("GE Tag: %s"), *Tag.ToString());

				// The Message GameplayTag is attached to GameplayEffect.
				FGameplayTag RequestTag = FGameplayTag::RequestGameplayTag(FName("Message"));
				if (Tag.MatchesTag(RequestTag))
				{
					if (const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag))
					{
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		}
	);

	AuraASC->AbilityEquippedDelegate.AddUObject(this, &ThisClass::OnAbilityEquipped);

	// Handle two case when Abilities given before or after callback bound.
	if (AuraASC->bStartupAbilitiesGiven)
	{
		OnInitializeStartupAbilities(AuraASC);
	}
	else
	{
		AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UAuraWidgetController::OnInitializeStartupAbilities);
	}
	
}

void UOverlayWidgetController::OnXPChanged(int32 NewXP) const 
{
	const AAuraPlayerState* AuraPS = GetAuraPlayerState();
	const ULevelUpInfo* LevelUpInfo = AuraPS->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Bluepirnt"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->MaxLevel();
	if (Level <= MaxLevel && Level > 0 )
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInfos[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInfos[Level - 1].LevelUpRequirement;
		const float XPBarPercent = static_cast<float>(NewXP - PreviousLevelUpRequirement) / static_cast<float>(LevelUpRequirement - PreviousLevelUpRequirement);
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	} 
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();

	FAuraAbilityInfo LastSlotInfo{};
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
