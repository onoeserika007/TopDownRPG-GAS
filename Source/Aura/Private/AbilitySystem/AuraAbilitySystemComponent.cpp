// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "GameplayTagContainer.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AuraLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::OnAbilityActorInfoSet()
{
	/** Called on server whenever a GE is applied to self. This includes instant and duration based GEs. */
	// so ThisClass::OnEffectApplied should be made a client RPC.
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::OnEffectApplied);

	const FAuraGameplayTags& AuraGameplayTags = FAuraGameplayTags::GetInstance();
	// AuraGameplayTags.Attributes_Secondary_Armor
}

void UAuraAbilitySystemComponent::AddCharacterAbilitiesFromSavedData(ULoadScreenSaveGame* SaveData)
{
	for(const auto& Data: SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> StartupAbilityClass = Data.GameplayAbility;
		FGameplayAbilitySpec LoadedAbilitySpec(StartupAbilityClass, Data.AbilityLevel);

		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilitySlot);
		LoadedAbilitySpec.DynamicAbilityTags.AddTag(Data.AbilityStatus);
		if (Data.AbilityType.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Type_Offensive))
		{
			GiveAbility(LoadedAbilitySpec);
		}
		else if (Data.AbilityType.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Type_Passive))
		{
			if (Data.AbilityStatus.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);
			}
			else
			{
				GiveAbility(LoadedAbilitySpec);
			}
		}
	}

	// So far Given Ability to ASC
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast(this);
}

// This only happens on server.
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for(auto AbilityClass: StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		if (auto* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
			// GiveAbilityAndActivateOnce(*const_cast<FGameplayAbilitySpec*>(&AbilitySpec));
		}
	}

	// So far Given Ability to ASC
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast(this);
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for(auto AbilityClass: StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
		AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		// DynamicAbilityTags contains the StartupInputTag
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// TODO: So What's the use of this two functions?
			AbilitySpecInputPressed(AbilitySpec);
			// What is the HELL for?
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		// DynamicAbilityTags contains the StartupInputTag
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			// TODO: So What's the use of this two functions?
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			// TODO: So What's the use of this two functions?
			AbilitySpecInputReleased(AbilitySpec);
			// And what is this hell for?
			InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, AbilitySpec.ActivationInfo.GetActivationPredictionKey());
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// Lock Abilities in case their status changed.
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (IsValid(AbilitySpec.Ability))
	{
		for (const auto& Tag: AbilitySpec.Ability->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities"))))
			{
				return Tag;
			}
		}
	}
	return {};
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (const auto& Tag: AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return {};
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (const auto& Tag: AbilitySpec.DynamicAbilityTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return Tag;
		}
	}
	return {};
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		if (AbilitySpec.Ability->AbilityTags.HasTagExact(AbilityTag))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusTagFromSpec(*Spec);
	}
	return {};
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return {};
}

// Attention!
// InputTag -- AbilitySpec.DynamicAbilityTags
// AbilityTag -- AbilitySpec.Ability->AbilityTags
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromInputTag(const FGameplayTag& InputTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec: GetActivatableAbilities())
	{
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag,
	bool bActivate)
{
	ActivatePassiveEffectDelegate.Broadcast(AbilityTag, bActivate);
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	// Minus one
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const auto& Info: AbilityInfo->AbilityInfos)
	{
		if (Level >= Info.LevelRequirement && Info.AbilityTag.IsValid())
		{
			// now this ability not in activatable abilities
			if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
			{
				FGameplayAbilitySpec AbilitySpec {Info.Ability, 1};
				AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::GetInstance().Abilities_Status_Eligible);
				GiveAbility(AbilitySpec);
				// Force to replicate
				MarkAbilitySpecDirty(AbilitySpec);
				ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::GetInstance().Abilities_Status_Eligible, AbilitySpec.Level);
			}
		}
	}
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
	}
	// At least it's not locked.
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const auto& GameplayTags = FAuraGameplayTags::GetInstance();
		FGameplayTag Status = GetStatusTagFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		MarkAbilitySpecDirty(*AbilitySpec);
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
	} 
}

void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& InputSlot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::GetInstance();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusTagFromSpec(*AbilitySpec);

		const bool bStatusValid = Status.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Status_Equipped)
		|| Status.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Status_Unlocked);
		if (bStatusValid)
		{
			// Handle activation/deactivation for passive abilities

			// SlotIsEmpty(slot)
			if (!SlotIsEmpty(InputSlot)) // Deactivate and clear its slot.
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecFromInputTag(InputSlot);
				if (SpecWithSlot)
				{
					const FGameplayTag& AbilityTagToBeReplaced = GetAbilityTagFromSpec(*SpecWithSlot);
					if (AbilityTag.MatchesTagExact(AbilityTagToBeReplaced)) // New and old are the same ability
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, InputSlot, PrevSlot);
						return;
					}

					if (IsPassiveAbility(*SpecWithSlot)) // Deactivate Old Passive Ability
					{
						MulticastActivatePassiveEffect(AbilityTagToBeReplaced, false);
						DeactivatePassiveAbilityDelegate.Broadcast(AbilityTagToBeReplaced);
						SpecWithSlot->DynamicAbilityTags.RemoveTag(GetStatusTagFromSpec(*SpecWithSlot));
						SpecWithSlot->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
					}

					// Clear this old ability's InputTag in AbilitySpec, just in case
					ClearInputSlot(AbilitySpec);
				}
			}

			if (!AbilityHasAnySlot(AbilitySpec)) // Ability doesn't yet have a slot (it's not active)
			{
				if (IsPassiveAbility(*AbilitySpec)) // if Passive, we are going to activate it in this case (it's not active)
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
			}
			
			// Remove this InputTag (slot) from any Ability has it.
			ClearAbilitiesOfInputSlot(InputSlot);

			// Assign to new slot
			AssignSlotToAbility(*AbilitySpec, InputSlot);
			
			// if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			// {
			// 	AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
			// 	AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			// }
			AbilitySpec->DynamicAbilityTags.RemoveTag(GetStatusTagFromSpec(*AbilitySpec));
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);

			MarkAbilitySpecDirty(*AbilitySpec);

			ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, InputSlot, PrevSlot);
		}
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, Status, Slot, PrevSlot);
}

void UAuraAbilitySystemComponent::ClearInputSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->DynamicAbilityTags.RemoveTag(Slot);
	MarkAbilitySpecDirty(*Spec);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfInputSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec: GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			ClearInputSlot(&Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot)
{
	return Spec->DynamicAbilityTags.HasTagExact(Slot);
}

bool UAuraAbilitySystemComponent::AbilityHasAnySlot(FGameplayAbilitySpec* Spec)
{
	return Spec->DynamicAbilityTags.HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& Spec: GetActivatableAbilities())
	{
		if (AbilityHasSlot(&Spec, Slot))
		{
			return false;
		}
	}
	return true;
}

bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	// only on server
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (AbilityInfo)
	{
		const FGameplayTag& AbilityTag = GetAbilityTagFromSpec(Spec);
		const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
		const FGameplayTag& AbilityType = Info.AbilityType;
		return AbilityType.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_Type_Passive);
	}
	return false;
}

void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearInputSlot(&Spec); // Clear old SlotTag from Ability to equip
	Spec.DynamicAbilityTags.AddTag(Slot);
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDesc,
                                                              FString& OutDescNextLevel)
{
	OutDesc = {};
	OutDescNextLevel = {};
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability);
		if (AuraAbility)
		{
			OutDesc = AuraAbility->GetDescription(AbilitySpec->Level);
			OutDescNextLevel = AuraAbility->GetDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::GetInstance().Abilities_None))
	{
		OutDesc = {};
	}
	else
	{
		OutDesc = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	return false;
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	// So far Given Ability to ASC
	// Replication is done every frame, so this only happens when all Abilities are given.
	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast(this);
	}
}

//  For Pickup Message
void UAuraAbilitySystemComponent::OnEffectApplied_Implementation(UAbilitySystemComponent* Asc, const FGameplayEffectSpec& EffectSpec,
                                                                 FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);
	EffectAssetTagsDelegate.Broadcast(TagContainer);
}

// For spell globe in spell menu.
void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}
