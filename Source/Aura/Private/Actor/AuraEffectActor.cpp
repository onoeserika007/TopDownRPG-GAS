// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AuraEffectActor.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "Components/SphereComponent.h"

// Sets default values
AAuraEffectActor::AAuraEffectActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
}

// Called when the game starts or when spawned
void AAuraEffectActor::BeginPlay()
{
	Super::BeginPlay();

}

void AAuraEffectActor::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (TScriptInterface<IAbilitySystemInterface> ASCInterface = OtherActor)
	{
		if (ASCInterface->GetAbilitySystemComponent())
		{
			// TODO: Use GameEffect, now use const cast for a hack.
			const UAuraAttributeSet* AuraAttributeSet = Cast<UAuraAttributeSet>(
				ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAuraAttributeSet::StaticClass())
				);
			const auto MutableAuraAttributeSet = const_cast<UAuraAttributeSet*>(AuraAttributeSet);
			MutableAuraAttributeSet->SetHealth(AuraAttributeSet->GetHealth() + 25.0);
			MutableAuraAttributeSet->SetMana(AuraAttributeSet->GetMana() + 25.0);
			Destroy();
		}
	}
}

void AAuraEffectActor::EndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}

