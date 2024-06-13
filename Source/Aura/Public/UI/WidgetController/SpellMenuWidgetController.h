// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraGameplayTags.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature,
	bool,bSpendPointsButtonEnabled,
	bool, bEquippedButtonEnabled,
	FString, DescriptionString,
	FString, NextDescriptionString);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityType);

struct FSelectedAbility
{
	FGameplayTag Ability {};
	FGameplayTag Status {};
};

struct FGameplayTag;
/**
 * 
 */
UCLASS()
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies();

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnPlayerStatChangedSignature SpellPointsChanged;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FWaitForEquipSelectionSignature WaitForEquipDelegate;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FWaitForEquipSelectionSignature StopWaitingForEquipDelegate;

	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;
	

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	UFUNCTION(BlueprintCallable)
	void SpellEquippedRowGlobePressed(const FGameplayTag& SlotInputTag, const FGameplayTag& AbilityType);

	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot);

	UFUNCTION(BlueprintCallable)
	void OnSpellMenuWidgetDestroyed();
private:
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton);
	FSelectedAbility SelectedAbility = {FAuraGameplayTags::GetInstance().Abilities_None, FAuraGameplayTags::GetInstance().Abilities_Status_Locked };
	int32 CurrentSpellPoints = 0;
	bool bWaitingForEquipSelection = false;
	FGameplayTag InputSlotOfSelectedAbility;
};
