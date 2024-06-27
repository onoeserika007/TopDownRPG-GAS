// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraBeamSpell.h"

#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"


void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& InHitResult)
{
	if (InHitResult.bBlockingHit)
	{
		MouseHitLocation = InHitResult.ImpactPoint;
		MouseHitActor = InHitResult.GetActor();
	}
	else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UAuraBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
			// ETraceTypeQuery::TraceTypeQuery1 -> Visibility??
			FHitResult HitResult{};
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter,
				SocketLocation,
				BeamTargetLocation,
				10.f,
				TraceTypeQuery1,
				false,
				ActorsToIgnore,
				EDrawDebugTrace::None,
				HitResult,
				true);

			if (HitResult.bBlockingHit)
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}

	// if (TScriptInterface<ICombatInterface> CombatInterface = MouseHitActor)
	// {
	// 	if (!CombatInterface->GetOnActorDeathDelegate().IsAlreadyBound(this, &ThisClass::UAuraBeamSpell::PrimaryTargetDied))
	// 	{
	// 		CombatInterface->GetOnActorDeathDelegate().AddDynamic(this, &ThisClass::UAuraBeamSpell::PrimaryTargetDied);
	// 	}
	// }
}

void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
	ActorsToIgnore.Add(MouseHitActor);

	TArray<AActor*> OverlappingActors;
	UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),
		OverlappingActors,
		ActorsToIgnore,
		850.f,
		MouseHitActor->GetActorLocation());

	int32 NumAdditionalTarget = FMath::Min(GetAbilityLevel() - 1, MaxNumShockTargets);
	// int32 NumAdditionalTarget = 5;
	if (bOverrideNumShockTargets)
	{
		NumAdditionalTarget = MaxNumShockTargets;
	}
	UAuraAbilitySystemLibrary::GetClosestTargets(NumAdditionalTarget, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());
}

void UAuraBeamSpell::BindPrimaryTargetDiedCallback(AActor* DeadActor)
{
	if (TScriptInterface<ICombatInterface> CombatInterface = DeadActor)
	{
		if (!CombatInterface->GetOnActorDeathDelegate().IsAlreadyBound(this, &ThisClass::UAuraBeamSpell::PrimaryTargetDied))
		{
			CombatInterface->GetOnActorDeathDelegate().AddDynamic(this, &ThisClass::UAuraBeamSpell::PrimaryTargetDied);
		}
	}
}

void UAuraBeamSpell::BindAdditionalTargetDiedCallback(AActor* DeadActor)
{
	if (TScriptInterface<ICombatInterface> CombatInterface = DeadActor)
	{
		if (!CombatInterface->GetOnActorDeathDelegate().IsAlreadyBound(this, &ThisClass::UAuraBeamSpell::AdditionalActorDied))
		{
			CombatInterface->GetOnActorDeathDelegate().AddDynamic(this, &ThisClass::UAuraBeamSpell::AdditionalActorDied);
		}
	}
}

void UAuraBeamSpell::UnbindAllTargetDiedCallback(AActor* DeadActor)
{
	if (TScriptInterface<ICombatInterface> CombatInterface = DeadActor)
	{
		CombatInterface->GetOnActorDeathDelegate().RemoveAll(this);
	}
}
