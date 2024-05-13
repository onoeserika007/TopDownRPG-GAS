// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerState.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"


AAuraPlayerState::AAuraPlayerState()
{
	NetUpdateFrequency = 100.0f;

	AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	/** For mixed Replication Mode
	 * The OwnerActor's Owner must be the Contoller. For Pawns, this is set automatically in PossessedBy().
	 * The PlayerState's Owner is automatically set to the Conttoller.
	 * Therefore, if your OwnerActor is not the PlayerState, and you use Mixed Replication Mode,
	 * you must call SetOwner() on the OwnerActor to set its owner to the Controller.
	 */
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// is automatically registered with the AbilitySystemComponent
	// you can't have more than one AttributeSet of the same class type
	// Since it takes up negligible memory, so you can even share a single attributeSet among all classes
	// Change attribute via GameplayEffect - client-side prediction is permitted.
	AttributeSet = CreateDefaultSubobject<UAuraAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* AAuraPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

TObjectPtr<UAttributeSet> AAuraPlayerState::GetAttributeSet() const
{
	return AttributeSet;
}
