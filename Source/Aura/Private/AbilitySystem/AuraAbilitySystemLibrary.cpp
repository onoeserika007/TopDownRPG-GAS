// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "GameplayAbilitySpec.h"
#include "AuraAbilityTypes.h"
#include "NiagaraValidationRule.h"
#include "Chaos/AABBTree.h"
#include "Interaction/CombatInterface.h"

bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject,
	FWidgetControllerParams& OutParams, AAuraHUD*& OutAuraHUD)
{
	if (const auto PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
		if (OutAuraHUD)
		{
			auto PS = PC->GetPlayerState<AAuraPlayerState>();
			auto ASC = PS->GetAbilitySystemComponent();
			auto AS = PS->GetAttributeSet();

			OutParams.PlayerController = PC;
			OutParams.PlayerState = PS;
			OutParams.AbilitySystemComponent = ASC;
			OutParams.AttributeSet = AS;
			return true;
		}
	}
	return false;
}

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams Params{};
	AAuraHUD* AuraHUD {};
	if (MakeWidgetControllerParams(WorldContextObject, Params, AuraHUD))
	{
		return AuraHUD->GetOverlayWidgetController(Params);
	}
	
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams Params{};
	AAuraHUD* AuraHUD {};
	if (MakeWidgetControllerParams(WorldContextObject, Params, AuraHUD))
	{
		return AuraHUD->GetAttributeMenuWidgetController(Params);
	}
	
	return nullptr;
}

USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams Params{};
	AAuraHUD* AuraHUD {};
	if (MakeWidgetControllerParams(WorldContextObject, Params, AuraHUD))
	{
		return AuraHUD->GetSpellMenuWidgetController(Params);
	}
	
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	// only valid on server
	UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(ClassInfo);

	FCharacterClassDefaultInfo CharacterDefaultInfo = ClassInfo->GetClassDefaultInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributeContextHandle = ASC->MakeEffectContext();
	PrimaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterDefaultInfo.PrimaryAttributes, Level, PrimaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data);

	FGameplayEffectContextHandle SecondaryAttributeContextHandle = ASC->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->SecondaryAttributes, Level, SecondaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data);

	FGameplayEffectContextHandle VitalAttributeContextHandle = ASC->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->VitalAttributes, Level, VitalAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data);
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	// only valid on Server
	UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(ClassInfo);

	for (auto AbilityClass: ClassInfo->CommonAbilities)
	{
		// Here we don't care about the level.
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}

	const FCharacterClassDefaultInfo& CharacterDefaultInfo = ClassInfo->GetClassDefaultInfo(CharacterClass);
	for (auto AbilityClass: CharacterDefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	// GameMode only valid on server
	auto AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode)
	{
		UCharacterClassInfo* ClassInfo = AuraGameMode->CharacterClassInfo;
		check(ClassInfo);

		return ClassInfo;
	}
	return nullptr;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	auto AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode)
	{
		UAbilityInfo* AbilityInfo = AuraGameMode->AbilityInfo;
		check(AbilityInfo);

		return AbilityInfo;
	}
	return nullptr;
}

bool UAuraAbilitySystemLibrary::IsBlockHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	// Since I will never use FGameplayEffectContextHandle base class anymore in this project,
	// the safety of this cast is ensured by the programmer.
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->IsBlockedHit();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
	const FVector& SphereOrigin)
{
	// Imitation of UGameplayStatics::ApplyRadialDamageWithFalloff
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	// query scene to see what we hit
	TArray<FOverlapResult> Overlaps;
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(
			Overlaps,
			SphereOrigin,
			FQuat::Identity,
			FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
			FCollisionShape::MakeSphere(Radius),
			SphereParams);
		for (FOverlapResult& OverlapResult: Overlaps)
		{
			const bool bImplementsCombatInterface = OverlapResult.GetActor()->Implements<UCombatInterface>();
			// You mush make sure that the Actor IMPLEMENTS the Interface before use it!!!
			if (bImplementsCombatInterface && !ICombatInterface::Execute_IsDead(OverlapResult.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(OverlapResult.GetActor()));
			}
		}
	}
}

bool UAuraAbilitySystemLibrary::IsFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool FirstIsPlayer = FirstActor->ActorHasTag(FName("Player"));
	const bool SecondIsPlayer = SecondActor->ActorHasTag(FName("Player"));
	const bool FirstIsEnemy = FirstActor->ActorHasTag(FName("Enemy"));
	const bool SecondIsEnemy = SecondActor->ActorHasTag(FName("Enemy"));

	const bool BothArePlayers = FirstIsPlayer && SecondIsPlayer;
	const bool BothAreEnemies = FirstIsEnemy && SecondIsEnemy;
	return BothArePlayers || BothAreEnemies;
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
	ECharacterClass CharacterClass, int32 CharacterLevel)
{
	// only valid on server
	UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(ClassInfo);

	const FCharacterClassDefaultInfo Info = ClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}
