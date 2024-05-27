 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "AbilityTask_TargetDataUnderMouse.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 
 */

UCLASS()
class AURA_API UAbilityTask_TargetDataUnderMouse : public UAbilityTask
{
    GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta=(
		DisplayName = "TargetDataUnderMouse",
		HidePin = "OwningAbility",
		DefaulttoSelf = "OwningAbility",
		BlueprintInternalUseOnly = "true"))
    static UAbilityTask_TargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

private:
	virtual void Activate() override;
	void SendMouseCursorData();

	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
public:
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

	
};
