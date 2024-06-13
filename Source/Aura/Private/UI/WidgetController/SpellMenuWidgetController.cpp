// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();

	// We don't have to use delegate to broadcast AbilityInfo asynchronously like in OverlayWidget,
	// since we should always have those abilities given to ASC gotten when we open SpellMenu.
	BroadcastAbilityInfo();
	SpellPointsChanged.Broadcast(GetAuraPlayerState()->GetSpellPoints());
}

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();

	GetAuraAbilitySystemComponent()->AbilityStatusChanged.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewSpellLevel)
		{
			// Update Button isEnabled
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
			{
				SelectedAbility.Status = StatusTag;
				bool bShouldEnableSpendPointsButton = false;
				bool bShouldEnableEquipButton = false;
				FString DescriptionString;
				FString NextDescriptionString;
				GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, DescriptionString, NextDescriptionString);
				ShouldEnableButtons(StatusTag, CurrentSpellPoints, bShouldEnableSpendPointsButton, bShouldEnableEquipButton);
				SpellGlobeSelectedDelegate.Broadcast(bShouldEnableSpendPointsButton, bShouldEnableEquipButton, DescriptionString, NextDescriptionString);
			}
			
			if (AbilityInfo)
			{
				FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
		);

	GetAuraAbilitySystemComponent()->AbilityEquippedDelegate.AddUObject(this, &ThisClass::OnAbilityEquipped);

	GetAuraPlayerState()->OnSpellPointsChangedDelegate.AddLambda([this](int32 NewSpellPoints)
	{
		SpellPointsChanged.Broadcast(NewSpellPoints);
		CurrentSpellPoints = NewSpellPoints;

		// Update Button isEnabled
		bool bShouldEnableSpendPointsButton = false;
		bool bShouldEnableEquipButton = false;
		FString DescriptionString;
		FString NextDescriptionString;
		GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, DescriptionString, NextDescriptionString);
		ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bShouldEnableSpendPointsButton, bShouldEnableEquipButton);
		SpellGlobeSelectedDelegate.Broadcast(bShouldEnableSpendPointsButton, bShouldEnableEquipButton, DescriptionString, NextDescriptionString);
	});

	
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	const int32 SpellPoints = GetAuraPlayerState()->GetSpellPoints();
	FGameplayTag AbilityStatus{};
	FGameplayAbilitySpec* AbilitySpec = GetAuraAbilitySystemComponent()->GetSpecFromAbilityTag(AbilityTag);
	
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_None);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = FAuraGameplayTags::GetInstance().Abilities_Status_Locked;
	}
	else
	{
		AbilityStatus = GetAuraAbilitySystemComponent()->GetStatusTagFromSpec(*AbilitySpec);
	}

	// Store
	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	FString DescriptionString;
	FString NextDescriptionString;
	GetAuraAbilitySystemComponent()->GetDescriptionsByAbilityTag(AbilityTag, DescriptionString, NextDescriptionString);
	bool bShouldEnableSpendPointsButton = false;
	bool bShouldEnableEquipButton = false;
	ShouldEnableButtons(AbilityStatus, SpellPoints, bShouldEnableSpendPointsButton, bShouldEnableEquipButton);
	SpellGlobeSelectedDelegate.Broadcast(bShouldEnableSpendPointsButton, bShouldEnableEquipButton, DescriptionString, NextDescriptionString);
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FAuraGameplayTags::GetInstance().Abilities_None;
	SelectedAbility.Status = FAuraGameplayTags::GetInstance().Abilities_Status_Locked;
	SpellGlobeSelectedDelegate.Broadcast(false, false, {}, {});
	
}

void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraAbilitySystemComponent())
	{
		GetAuraAbilitySystemComponent()->ServerSpendSpellPoint(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	const FGameplayTag SelectedStatus = GetAuraAbilitySystemComponent()->GetStatusFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped))
	{
		InputSlotOfSelectedAbility = GetAuraAbilitySystemComponent()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::SpellEquippedRowGlobePressed(const FGameplayTag& SlotInputTag,
	const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;;

	GetAuraAbilitySystemComponent()->ServerEquipAbility(SelectedAbility.Ability, SlotInputTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	bWaitingForEquipSelection = false;

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

	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
}

void USpellMenuWidgetController::OnSpellMenuWidgetDestroyed()
{
	bWaitingForEquipSelection = false;
}


void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,
                                                     bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton)
{
	const auto& GameplayTags = FAuraGameplayTags::GetInstance();
	bShouldEnableSpendPointsButton = false;
	bShouldEnableEquipButton = false;
	
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		bShouldEnableEquipButton = false;
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpendPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Locked))
	{
		bShouldEnableEquipButton = false;
		bShouldEnableSpendPointsButton = false;
	}
}
