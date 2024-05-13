// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Widget/AuraUserWidget.h"

void UAuraUserWidget::SetWidgetController(UObject* InWidgetController)
{
	WidgetController = InWidgetController;
	UE_LOG(LogTemp, Warning, TEXT("UAuraUserWidget::SetWidgetController - controller set."));
	OnWidgetControllerSet();
}
