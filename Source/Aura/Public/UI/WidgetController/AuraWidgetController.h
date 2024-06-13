// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

class UAbilityInfo;
class UAuraAttributeSet;
class UAuraAbilitySystemComponent;
class AAuraPlayerState;
class AAuraPlayerController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, Info);

class UAbilitySystemComponent;
class UAttributeSet;

USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()
	FWidgetControllerParams() = default;
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
		:PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent>  AbilitySystemComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet>  AttributeSet;
};
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UAuraWidgetController : public UObject
{
public:
	UFUNCTION(BlueprintPure)
	AAuraPlayerController* GetAuraPlayerController() const;
	AAuraPlayerState* GetAuraPlayerState() const;
	UAuraAbilitySystemComponent* GetAuraAbilitySystemComponent() const;
	UAuraAttributeSet* GetAuraAttributeSet() const;

private:
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& Params);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	
	virtual void BindCallbacksToDependencies();

	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	void BroadcastAbilityInfo() const;
	void OnInitializeStartupAbilities(UAuraAbilitySystemComponent* AuraASC) const;
	
protected:
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent>  AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet>  AttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category ="Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;
	
private:
	mutable TObjectPtr<AAuraPlayerController> AuraPlayerController;
	mutable TObjectPtr<AAuraPlayerState> AuraPlayerState;
	mutable TObjectPtr<UAuraAbilitySystemComponent>  AuraAbilitySystemComponent;
	mutable TObjectPtr<UAuraAttributeSet>  AuraAttributeSet;
};
