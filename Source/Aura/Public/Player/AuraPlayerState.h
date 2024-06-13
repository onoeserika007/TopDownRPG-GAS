// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "AuraPlayerState.generated.h"

class ULevelUpInfo;
class UAbilitySystemComponent;
class UAttributeSet;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)
/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	/*
	 * Functions
	 */
public:
	AAuraPlayerState();

	// IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	TObjectPtr<UAttributeSet> GetAttributeSet() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	int32 GetPlayerLevel() const { return Level; }
	int32 GetXP() const { return XP; }
	int32 GetAttributePoints() const { return AttributePoints; }
	int32 GetSpellPoints() const { return SpellPoints; }
	
	void AddToXP(int32 InXP);
	void AddToLevel(int32 InLevel);
	void AddToAttributePoints(int32 InPoints);
	void AddToSpellPoints(int32 InPoints);
	void SetXP(int32 InXP);
	void SetLevel(int32 InLevel);
protected:

private:
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);

	/*
	 * Attributes
	 */
public:
	FOnPlayerStatChanged OnXPChangedDelegate;
	FOnPlayerStatChanged OnLevelChangedDelegate;
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
private:
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_XP)
	int32 XP = 0;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 1;

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing=OnRep_SpellPoints)
	int32 SpellPoints = 0;
};
