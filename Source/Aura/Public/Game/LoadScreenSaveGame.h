// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;

UENUM(BlueprintType)
enum ESaveSlotStatus: int8
{
	Vacant,
	EnterName,
	Taken,
	DefaultMax
};

USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType {};

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 1;

	friend inline bool operator==(const FSavedAbility& Lhs, const FSavedAbility& Rhs)
	{
		return Lhs.AbilityTag == Rhs.AbilityTag;
	}
};

USTRUCT(BlueprintType)
struct FSavedActor
{
	GENERATED_BODY()

	UPROPERTY()
	FName ActorName {};

	UPROPERTY()
	FTransform Transform {};

	// Serialized variables from the actor - only those marked with SaveGame specifier
	UPROPERTY()
	TArray<uint8> Bytes;

	friend inline bool operator==(const FSavedActor Lhs, const FSavedActor Rhs)
	{
		return Lhs.ActorName == Rhs.ActorName;
	}
};

USTRUCT(BlueprintType)
struct FSavedMap
{
	GENERATED_BODY()

	UPROPERTY()
	FString MapAssetName {};

	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};

/**
 * 
 */
UCLASS()
class AURA_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString SlotName {};

	UPROPERTY()
	int32 SlotIndex = 0;

	UPROPERTY()
	FString PlayerName { "Default Name" };

	UPROPERTY()
	FString MapName { "Default Map Name" }; // Display to Screen

	UPROPERTY()
	FString MapAssetName { "Default Map Asset Name" };

	UPROPERTY()
	FName PlayerStartTag;

	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus {ESaveSlotStatus::Vacant};

	UPROPERTY()
	bool bFirstTimeLoadIn = true;

	/* Player */
	UPROPERTY()
	int32 PlayerLevel = 1;

	UPROPERTY()
	int32 XP = 0;

	UPROPERTY()
	int32 SpellPoints = 0;

	UPROPERTY()
	int32 AttributePoints = 0;

	UPROPERTY()
	float Strength = 0;
	
	UPROPERTY()
	float Intelligence = 0;

	UPROPERTY()
	float Resilience = 0;

	UPROPERTY()
	float Vigor = 0;

	/* Abilities */
	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities {};

	/* Map */
	UPROPERTY()
	TArray<FSavedMap> SavedMaps {};

	FSavedMap GetSavedMapWithMapName(const FString& InMapName) const;
	bool HasMap(const FString& InMapName);
};
