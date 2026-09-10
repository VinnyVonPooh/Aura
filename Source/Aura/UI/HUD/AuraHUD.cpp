// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraHUD.h"

#include "Aura/UI/Widget/AuraUserWidget.h"
#include "Blueprint/UserWidget.h"

void AAuraHUD::BeginPlay()
{
	Super::BeginPlay();
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	Widget->AddToViewport();
}