// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "GameplayAbilitySpec.h"
#include "AuraAbilityTypes.h"
#include "AuraGameplayTags.h"
#include "NiagaraValidationRule.h"
#include "Chaos/AABBTree.h"
#include "Game/LoadScreenSaveGame.h"
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

void UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject,
	UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	UCharacterClassInfo* ClassInfo = GetCharacterClassInfo(WorldContextObject);
	check(ClassInfo);

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
	const AActor* SourceAvatarActor = ASC->GetAvatarActor();
	FGameplayEffectContextHandle EffectContextHandle = ASC->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor); // Source Object Added
	
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContextHandle);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);

	// Primary
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// Secondary
	FGameplayEffectContextHandle SecondaryAttributeContextHandle = ASC->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->SecondaryAttributes_Infinite_Aura, 1.f, SecondaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data);

	// Vital
	FGameplayEffectContextHandle VitalAttributeContextHandle = ASC->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassInfo->VitalAttributes, 1.f, VitalAttributeContextHandle);
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

// only on server
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

bool UAuraAbilitySystemLibrary::HasSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->HasSuccessfulDebuff();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->IsDebuff();
	}
	return false;
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetDamageType();
	}
	return {};
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetDeathImpulse();
	}
	return {};
}

FVector UAuraAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetKnockbackForce();
	}
	return {};
}

bool UAuraAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->IsRadialDamage();
	}
	return {};
}

float UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetRadialDamageInnerRadius();
	}
	return {};
}

float UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetRadialDamageOuterRadius();
	}
	return {};
}

FVector UAuraAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		return AuraEffectContext->GetRadialDamageOrigin();
	}
	return {};
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

void UAuraAbilitySystemLibrary::SetHasSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInHasSuccessfulDebuff)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetHasSuccessfulDebuff(bInHasSuccessfulDebuff);
	}
}

void UAuraAbilitySystemLibrary::SetIsDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsDebuff)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetIsDebuff(bInIsDebuff);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDamage)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetDebuffDamage(InDebuffDamage);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle,
	float InDebuffDuration)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetDebuffDuration(InDebuffDuration);
	}
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	float InDebuffFrequency)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetDebuffFrequency(InDebuffFrequency);
	}
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetDamageType(InDamageType);
	}
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InDeathImpulse)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetDeathImpulse(InDeathImpulse);
	}
}

void UAuraAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InKnockbackForce)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetKnockbackForce(InKnockbackForce);
	}
}

void UAuraAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
	bool InIsRadialDamage)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetIsRadialDamage(InIsRadialDamage);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InRadialDamageInnerRadius)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetRadialDamageInnerRadius(InRadialDamageInnerRadius);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InRadialDamageOuterRadius)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetRadialDamageOuterRadius(InRadialDamageOuterRadius);
	}
}

void UAuraAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InRadialDamageOrigin)
{
	FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get());
	if (AuraEffectContext)
	{
		AuraEffectContext->SetRadialDamageOrigin(InRadialDamageOrigin);
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

void UAuraAbilitySystemLibrary::GetClosestTargets(const int32 MaxTargets, const TArray<AActor*>& Actors,
                                                  TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while(NumTargetsFound < MaxTargets)
	{
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor = nullptr;
		for (auto PotentialTarget: ActorsToCheck)
		{
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.Add(ClosestActor);
		++NumTargetsFound;
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

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(
	const FDamageEffectParams& DamageEffectParams)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	// just attach these data on Context
	// AssignTagSetByCaller and Context are parallel in fact.
	UAuraAbilitySystemLibrary::SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
	UAuraAbilitySystemLibrary::SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);
	UAuraAbilitySystemLibrary::SetDamageType(EffectContextHandle, DamageEffectParams.DamageType);
	UAuraAbilitySystemLibrary::SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	UAuraAbilitySystemLibrary::SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContextHandle;
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

TArray<FVector> UAuraAbilitySystemLibrary::SpawnEvenlySpacedLocations(const UObject* WorldContextObject, const FVector& Forward, const FVector& Location,
	float SpreadAngle, int32 SpawnNums, float MinSpawnDistance, float MaxSpawnDistance)
{
	const float DeltaSpread = SpreadAngle / SpawnNums;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpreadAngle / 2.f, FVector::UpVector);
	UKismetSystemLibrary::DrawDebugArrow(
		WorldContextObject,
		Location + LeftOfSpread * MinSpawnDistance,
		Location + LeftOfSpread * MaxSpawnDistance,
		4.f, 
		FLinearColor::Green,
		3.f);

	TArray<FVector> SpawnLocations;
	for (int i = 0; i < SpawnNums; i++)
	{
		const FVector Division = LeftOfSpread.RotateAngleAxis(i * DeltaSpread, FVector::UpVector);
		UKismetSystemLibrary::DrawDebugArrow(
		WorldContextObject,
		Location + Division * MinSpawnDistance,
		Location + Division * MaxSpawnDistance,
		4.f, 
		FLinearColor::Blue,
		3.f);
		
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread / 2.f + i * DeltaSpread, FVector::UpVector);
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		// Force on the ground
		FHitResult Hit;
		WorldContextObject->GetWorld()->LineTraceSingleByChannel(
			Hit,
			ChosenSpawnLocation + FVector(0.f, 0.f, 400.f),
			ChosenSpawnLocation - FVector(0.f, 0.f, 400.f),
			ECC_Visibility
			);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
			SpawnLocations.Add(ChosenSpawnLocation);
		}
	}

	const FVector RightOfSpread = Forward.RotateAngleAxis(SpreadAngle / 2.f, FVector::UpVector);
	UKismetSystemLibrary::DrawDebugArrow(
		WorldContextObject,
		Location + RightOfSpread * MinSpawnDistance,
		Location + RightOfSpread * MaxSpawnDistance,
		4.f, 
		FLinearColor::Red,
		3.f);

	return SpawnLocations;
}

TArray<FRotator> UAuraAbilitySystemLibrary::SpawnEvenlySpacedRotators(const UObject* WorldContextObject,
	const FVector& Forward, const FVector& Axis, float SpreadAngle, int32 SpawnNums)
{
	const float DeltaSpread = SpreadAngle / SpawnNums;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpreadAngle / 2.f, Axis);
	
	TArray<FRotator> SpawnRotators;
	for (int i = 0; i < SpawnNums; i++)
	{
		const FVector Division = LeftOfSpread.RotateAngleAxis(i * DeltaSpread, Axis);
		
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread / 2.f + i * DeltaSpread, Axis);
		SpawnRotators.Add(Direction.Rotation());
	}
	
	const FVector RightOfSpread = Forward.RotateAngleAxis(SpreadAngle / 2.f, Axis);

	return SpawnRotators;
}


TArray<FVector> UAuraAbilitySystemLibrary::SpawnEvenlyRotatedVectors(const UObject* WorldContextObject,
                                                                     const FVector& Forward, const FVector& Axis, float SpreadAngle, int32 SpawnNums)
{
	TArray<FVector> SpawnedVectors;
	const auto SpawnedRotators = UAuraAbilitySystemLibrary::SpawnEvenlySpacedRotators(WorldContextObject, Forward, Axis, SpreadAngle, SpawnNums);
	for (const auto& Rot: SpawnedRotators)
	{
		SpawnedVectors.Add(Rot.Vector());
	}
	return SpawnedVectors;
}

void UAuraAbilitySystemLibrary::SetRadialDamageEffectParam(FDamageEffectParams& DamageEffectParams, bool InIsRadial,
	float InInnerRadius, float InOuterRadius, FVector InOrigin)
{
	DamageEffectParams.bIsRadialDamage = InIsRadial;
	DamageEffectParams.RadialDamageInnerRadius = InInnerRadius;
	DamageEffectParams.RadialDamageOuterRadius = InOuterRadius;
	DamageEffectParams.RadialDamageOrigin = InOrigin;
}

void UAuraAbilitySystemLibrary::SetImpulseDamageEffectParam(FDamageEffectParams& DamageEffectParams,
	FVector ImpulseDirection, FVector KnockbackDirection, float ImpulseMagnitude, float KnockbackMagnitude)
{
	if (ImpulseMagnitude >= 0.f)
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection.GetSafeNormal() * ImpulseMagnitude;
	}
	else
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection.GetSafeNormal() * DamageEffectParams.DeathImpulseMagnitude;
	}

	if (KnockbackMagnitude >= 0.f)
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection.GetSafeNormal() * KnockbackMagnitude;
	}
	else
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection.GetSafeNormal() * DamageEffectParams.KnockbackForceMagnitude;
	}
}

void UAuraAbilitySystemLibrary::SetTargetDamageEffectParam(FDamageEffectParams& DamageEffectParams, AActor* TargetActor)
{
	DamageEffectParams.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
} 



