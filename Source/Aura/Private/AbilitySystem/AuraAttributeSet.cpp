// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "AuraGameplayTags.h"
#include "LocalizationDescriptor.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AuraLogChannels.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"
#include <memory>

#include "AuraAbilityTypes.h"

#define ADD_TAG_ATTRIBUTE_PAIR(AttributeName, Category, TagName) \
	TagsToAttributes.Add(GameplayTags.Attributes_##Category##_##TagName, Get##AttributeName##Attribute);

UAuraAttributeSet::UAuraAttributeSet()
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();

	// TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength, GetStrengthAttribute);
	// primary
	ADD_TAG_ATTRIBUTE_PAIR(Strength, Primary, Strength);
	ADD_TAG_ATTRIBUTE_PAIR(Intelligence, Primary, Intelligence);
	ADD_TAG_ATTRIBUTE_PAIR(Resilience, Primary, Resilience);
	ADD_TAG_ATTRIBUTE_PAIR(Vigor, Primary, Vigor);

	ADD_TAG_ATTRIBUTE_PAIR(Armor, Secondary, Armor);
	ADD_TAG_ATTRIBUTE_PAIR(ArmorPenetration, Secondary, ArmorPenetration);
	ADD_TAG_ATTRIBUTE_PAIR(BlockChance, Secondary, BlockChance);
	ADD_TAG_ATTRIBUTE_PAIR(CriticalHitChance, Secondary, CriticalHitChance);
	ADD_TAG_ATTRIBUTE_PAIR(CriticalHitDamage, Secondary, CriticalHitDamage);
	ADD_TAG_ATTRIBUTE_PAIR(CriticalHitResistance, Secondary, CriticalHitResistance);
	ADD_TAG_ATTRIBUTE_PAIR(HealthRegeneration, Secondary, HealthRegeneration);
	ADD_TAG_ATTRIBUTE_PAIR(ManaRegeneration, Secondary, ManaRegeneration);
	ADD_TAG_ATTRIBUTE_PAIR(MaxHealth, Secondary, MaxHealth);
	ADD_TAG_ATTRIBUTE_PAIR(MaxMana, Secondary, MaxMana);

	ADD_TAG_ATTRIBUTE_PAIR(FireResistance, Resistance, Fire);
	ADD_TAG_ATTRIBUTE_PAIR(LightningResistance, Resistance, Lightning);
	ADD_TAG_ATTRIBUTE_PAIR(ArcaneResistance, Resistance, Arcane);
	ADD_TAG_ATTRIBUTE_PAIR(PhysicalResistance, Resistance, Physical);
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);
}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		// This doesn't change the modifier, just the value returned from querying the modifier.
		// Later operations recalculate the current value from all modifiers.
		// We need to clamp again.
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
	}

	if (Attribute == GetManaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
	}
}

void UAuraAttributeSet::SetEffectProperties(const FGameplayEffectModCallbackData& Data, FEffectProperties& Props)
{
	// Source = causer of the effect, Target = target of the effect (Owner of the AS)
	const FGameplayEffectContextHandle EffectContextHandle = Data.EffectSpec.GetContext();
	Props.EffectContextHandle = EffectContextHandle;

	const auto SourceAsc = EffectContextHandle.GetOriginalInstigatorAbilitySystemComponent();
	if (SourceAsc && SourceAsc->AbilityActorInfo)
	{
		AActor* SourceAvatarActor = SourceAsc->AbilityActorInfo->AvatarActor.Get();
		AController* SourcePlayerController = SourceAsc->AbilityActorInfo->PlayerController.Get();
		if (!SourcePlayerController && SourceAvatarActor)
		{
			if (const APawn* Pawn = Cast<APawn>(SourceAvatarActor))
			{
				SourcePlayerController = Pawn->GetController();
			}
		}
		ACharacter* SourceCharacter = Cast<ACharacter>(SourceAvatarActor);

		Props.SourceAsc = SourceAsc;
		Props.SourceAvatarActor = SourceAvatarActor;
		Props.SourceCharacter = SourceCharacter;
		Props.SourceController = SourcePlayerController;
	}

	if (Data.Target.AbilityActorInfo)
	{
		AActor* TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		AController* TargetPlayerController = Data.Target.AbilityActorInfo->PlayerController.Get();
		ACharacter* TargetCharacter = Cast<ACharacter>(TargetAvatarActor);
		const auto TargetAsc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetAvatarActor);

		Props.TargetAsc = TargetAsc;
		Props.TargetAvatarActor = TargetAvatarActor;
		Props.TargetCharacter = TargetCharacter;
		Props.TargetController = TargetPlayerController;
	}
}

void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		if (ICombatInterface::Execute_IsDead(Props.TargetCharacter))
		{
			return;
		}
	}

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("Changed Health on %s, Health: %f"), *Props.TargetAvatarActor->GetName(), GetHealth());
	}

	if (Data.EvaluatedData.Attribute == GetManaAttribute())
	{
		SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
	}

	if (Data.EvaluatedData.Attribute == GetArmorAttribute())
	{
		
	}

	if (Data.EvaluatedData.Attribute == GetStrengthAttribute())
	{
		
	}
 
	// Meta Attribute IncomingDamage
	// Since it won't be replicated, and we only set it on the server(????where?)
	// PS: we only apply the GE of Fire-bolt on server
	// so this if block only happens on server.
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		HandleIncomingDamage(Props);
	}

	if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
	{
		HandleIncomingXP(Props);
	}
}

void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
	const float LocalIncomingDamage = GetIncomingDamage();
	SetIncomingDamage(0.0f); // consume
	if (LocalIncomingDamage > 0.f)
	{
		const float NewHealth = GetHealth() - LocalIncomingDamage;
		SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));

		const bool bFatal = NewHealth <= 0.0f;
		if (bFatal)
		{
			// TODO: Death Impulse
			TScriptInterface<ICombatInterface> CombatInterface = Props.TargetAvatarActor;
			if (CombatInterface)
			{
				CombatInterface->Die(UAuraAbilitySystemLibrary::GetDeathImpulse(Props.EffectContextHandle));
			}
			SendXPEvent(Props);
		}
		else
		{
			const bool bIsDebuff = UAuraAbilitySystemLibrary::IsDebuff(Props.EffectContextHandle);
			const bool bIsLightingDamage = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle).MatchesTagExact(FAuraGameplayTags::GetInstance().Damage_Lightning);
			if (!bIsDebuff && !bIsLightingDamage) // if damage from debuff, do not hit react
			{
				/// Trying to Activate HitReact Ability!!!!
				FGameplayTagContainer TagContainer;
				TagContainer.AddTag(FAuraGameplayTags::GetInstance().Effects_HitReact);
				Props.TargetAsc->TryActivateAbilitiesByTag(TagContainer);

				const FVector& KnockbackForce = UAuraAbilitySystemLibrary::GetKnockbackForce(Props.EffectContextHandle);
				if (!KnockbackForce.IsNearlyZero(1.f))
				{
					Props.TargetCharacter->LaunchCharacter(KnockbackForce, true, true);
				}
			}

			// if (bIsLightingDamage)
			// {
			// 	if (Props.TargetCharacter && Props.TargetCharacter->Implements<UCombatInterface>())
			// 	{
			// 		ICombatInterface::Execute_SetIsBeingShocked(Props.TargetCharacter, true);
			// 	}
			// }
		}

		const bool bBlock = UAuraAbilitySystemLibrary::IsBlockHit(Props.EffectContextHandle);
		const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
		ShowFloatingText(Props, LocalIncomingDamage, bBlock, bCriticalHit);

		// Handle debuff
		if (UAuraAbilitySystemLibrary::HasSuccessfulDebuff(Props.EffectContextHandle))
		{
			HandleDebuff(Props);
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					10,
					FColor::Yellow,
					FString("Debuff Added."));
			}
		}
	}
}

void UAuraAttributeSet::HandleDebuff(const FEffectProperties& Props)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
	FGameplayEffectContextHandle EffectContextHandle = Props.SourceAsc->MakeEffectContext();
	EffectContextHandle.AddSourceObject(Props.SourceAvatarActor);

	const FGameplayTag DebuffDamageType = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);
	const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);
	const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);
	const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);

	FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DebuffDamageType.ToString());
	UGameplayEffect* Effect = NewObject<UGameplayEffect>(GetTransientPackage(), FName(DebuffName));

	Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
	Effect->Period = DebuffFrequency;
	Effect->DurationMagnitude = FScalableFloat(DebuffDuration);

	// Granted Tags
	const FGameplayTag DebuffTag = GameplayTags.DamageTypesToDebuffs[DebuffDamageType];
	Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.DamageTypesToDebuffs[DebuffDamageType]);
	if (DebuffTag.MatchesTagExact(GameplayTags.Debuff_Stun))
	{
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_CursorTrace);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputHeld);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputPressed);
		Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputReleased);
	}

	Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
	Effect->StackLimitCount = 1;

	const int32 InIndex = Effect->Modifiers.Num();
	Effect->Modifiers.Add(FGameplayModifierInfo());
	FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[InIndex];
	ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
	ModifierInfo.ModifierOp = EGameplayModOp::Additive;
	ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();

	if (std::shared_ptr<FGameplayEffectSpec> MutableSpec = std::make_shared<FGameplayEffectSpec>(Effect, EffectContextHandle, 1.f))
	{
		FAuraGameplayEffectContext* AuraContext = static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
		AuraContext->SetDamageType(DebuffDamageType);
		AuraContext->SetIsDebuff(true);

		Props.TargetAsc->ApplyGameplayEffectSpecToSelf(*MutableSpec);
	}
}

void UAuraAttributeSet::HandleIncomingXP(const FEffectProperties& Props)
{
	const float LocalIncomingXP = GetIncomingXP();
	SetIncomingXP(0.f);
	UE_LOG(LogAura, Log, TEXT("Incoming XP: %f"), LocalIncomingXP);

	// TODO: See if we should level up
	// Source Character is the owner, since GA_ListenForEvents applies GE_EventBasedEffect, adding to IncomingXP.
	if (Props.SourceCharacter->Implements<UPlayerInterface>() && Props.SourceCharacter->Implements<UCombatInterface>())
	{
		const int32 CurrentLevel = ICombatInterface::Execute_GetPlayerLevel(Props.SourceCharacter);
		const int32 CurrentXP = IPlayerInterface::Execute_GetXP(Props.SourceCharacter);

		const int32 NewLevel = IPlayerInterface::Execute_FindLevelForXP(Props.SourceCharacter, CurrentXP + LocalIncomingXP);
		int32 NumLevelUps = NewLevel - CurrentLevel;
		while (NumLevelUps > 0)
		{
			int32 AttributePointsReward = IPlayerInterface::Execute_GetAttributePointsReward(Props.SourceCharacter, CurrentLevel);
			int32 SpellPointReward = IPlayerInterface::Execute_GetSpellPointsReward(Props.SourceCharacter, CurrentLevel);

			IPlayerInterface::Execute_AddToPlayerLevel(Props.SourceCharacter, 1);
			IPlayerInterface::Execute_AddToAttributePoints(Props.SourceCharacter, AttributePointsReward);
			IPlayerInterface::Execute_AddToSpellPoints(Props.SourceCharacter, SpellPointReward);

			// Handle level up effects
			IPlayerInterface::Execute_LevelUp(Props.SourceCharacter);
			// SetHealth(GetMaxHealth());
			// SetMana(GetMaxMana());
			bTopOffHealth = true;
			bTopOffMana = true;
			NumLevelUps--;
		}
		IPlayerInterface::Execute_AddToXP(Props.SourceCharacter, LocalIncomingXP);
	}
		
}

void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);

	if (Attribute == GetMaxHealthAttribute() && bTopOffHealth)
	{
		SetHealth(GetMaxHealth());
		bTopOffHealth = false; // Consume that boolean.
	}

	if (Attribute == GetMaxManaAttribute() && bTopOffMana)
	{
		SetMana(GetMaxMana());
		bTopOffMana = false;
	}
}

void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit, bool bCriticalHit)
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		// Show Damage Text for both Source and Victim
		if (Props.SourceCharacter)
		{
			if (auto PC = Cast<AAuraPlayerController>(Props.SourceCharacter->GetController()) )
			{
				PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
			}
		}

		if (Props.TargetCharacter)
		{
			if (auto PC = Cast<AAuraPlayerController>(Props.TargetCharacter->GetController()) )
			{
				PC->ShowDamageNumber(Damage, Props.TargetCharacter, bBlockedHit, bCriticalHit);
			}
		}
		
	}
}

void UAuraAttributeSet::SendXPEvent(const FEffectProperties& Props)
{
	if (Props.TargetCharacter->Implements<UCombatInterface>())
	{
		const int32 TargetLevel = ICombatInterface::Execute_GetPlayerLevel(Props.TargetCharacter);
		const ECharacterClass TargetClass = ICombatInterface::Execute_GetCharacterClass(Props.TargetCharacter);
		const int32 XPReward = UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(Props.TargetCharacter, TargetClass, TargetLevel);

		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
		FGameplayEventData Payload;
		Payload.EventTag = GameplayTags.Attributes_Meta_IncomingXP;
		Payload.EventMagnitude = XPReward;
		// Source: Who cause the damage.
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Props.SourceCharacter, GameplayTags.Attributes_Meta_IncomingXP, Payload);
	}
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);
}

void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);
}

void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}

void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}

void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}

void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance) const
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}



