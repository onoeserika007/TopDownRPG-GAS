// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/HUD/AuraHUD.h"
#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& Params)
{
	if (!IsValid(OverlayWidgetController))
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(Params);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& Params)
{
	if (!IsValid(AttributeMenuWidgetController))
	{
		check(AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(Params);
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

USpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& Params)
{
	if (!IsValid(SpellMenuWidgetController))
	{
		check(SpellMenuWidgetControllerClass);
		// Maybe it's quite important for objects generated from NewObject func to be received by pointer wrapped with UPROPERTY()
		// Maybe raw pointer will not prevent it to be collected by GC.
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
		SpellMenuWidgetController->SetWidgetControllerParams(Params);
		SpellMenuWidgetController->BindCallbacksToDependencies();
	}
	return SpellMenuWidgetController;
}

void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class unassigned, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class unassigned, please fill out BP_AuraHUD"));

	if (UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass))
	{
		OverlayWidget = Cast<UAuraUserWidget>(Widget);

		FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
		UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);
		OverlayWidget->SetWidgetController(WidgetController);
		WidgetController->BroadcastInitialValues();
		
		OverlayWidget->AddToViewport();
	}
}

void AAuraHUD::BeginPlay()
{
	Super::BeginPlay();
}
