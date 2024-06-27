// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/AuraGameInstance.h"
#include "Game/AuraGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

#define CONCATENATE(x, y) x##y
#define TO_STRING(x) #x

#define INIT_LOAD_SLOT_WITH_INDEX(index) \
	LoadSlot_##index = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);\
	LoadSlots.Add(##index, LoadSlot_##index);\
	LoadSlot_##index->LoadSlotName = FString(TO_STRING(LoadSlot_##index));\
	LoadSlot_##index->SlotIndex = index;

void UMVVM_LoadScreen::InitializeSlots()
{
	INIT_LOAD_SLOT_WITH_INDEX(0);
	INIT_LOAD_SLOT_WITH_INDEX(1);
	INIT_LOAD_SLOT_WITH_INDEX(2);
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	if (LoadSlots.Contains(Index))
	{
		return LoadSlots[Index];
	}
	return nullptr;
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, FString EnteredName)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));

	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SetMapName(AuraGameMode->DefaultMapName);
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->LoadSlotStatus = ESaveSlotStatus::Taken;
	LoadSlots[Slot]->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
	

	AuraGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();

	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
	AuraGameInstance->LoadSlotName = LoadSlots[Slot]->LoadSlotName;
	AuraGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	AuraGameInstance->PlayerStartTag = LoadSlots[Slot]->PlayerStartTag;
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndexDelegate.Broadcast(ESaveSlotStatus::EnterName);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelectedDelegate.Broadcast();
	for (const auto [SlotIndex, LoadSlot]: LoadSlots)
	{
		if (SlotIndex == Slot)
		{
			LoadSlot->EnableSelectSlotButtonDelegate.Broadcast(false);
		}
		else
		{
			LoadSlot->EnableSelectSlotButtonDelegate.Broadcast(true);
		}
	}
	SelectedSlot = LoadSlots[Slot];
}

void UMVVM_LoadScreen::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	for (const auto [SlotIndex, LoadSlot]: LoadSlots)
	{
		ULoadScreenSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlot->LoadSlotName, SlotIndex);

		LoadSlot->LoadSlotStatus = SaveObject->SaveSlotStatus;
		LoadSlot->PlayerStartTag = SaveObject->PlayerStartTag;
		LoadSlot->SetPlayerName(SaveObject->PlayerName);
		LoadSlot->SetMapName(SaveObject->MapName);
		LoadSlot->SetPlayerLevel(SaveObject->PlayerLevel);
		LoadSlot->InitializeSlot();
	}
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if(IsValid(SelectedSlot))
	{
		AAuraGameModeBase::DeleteSlotData(SelectedSlot, SelectedSlot->SlotIndex);
		SelectedSlot->LoadSlotStatus = ESaveSlotStatus::Vacant;
		SelectedSlot->InitializeSlot();
	}
}

void UMVVM_LoadScreen::UndoSelectedSlot()
{
	if (IsValid(SelectedSlot))
	{
		SelectedSlot->EnableSelectSlotButtonDelegate.Broadcast(true);
		SelectedSlot = nullptr;
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (IsValid(AuraGameMode) && IsValid(SelectedSlot))
	{
		AuraGameMode->TravelToMapBySlot(SelectedSlot);

		UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
		AuraGameInstance->LoadSlotName = SelectedSlot->LoadSlotName;
		AuraGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
		AuraGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	}
}

