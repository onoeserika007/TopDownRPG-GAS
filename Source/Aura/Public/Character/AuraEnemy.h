// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacter.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacter, public IEnemyInterface
{
	GENERATED_BODY()
public:
	/**
	* IEnemyInterface
	*/
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;

	AAuraEnemy();

	UPROPERTY(BlueprintReadOnly)
	bool bHighlighted = false;

protected:
	void BeginPlay() override;
};