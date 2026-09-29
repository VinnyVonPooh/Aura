// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemLibrary.h"

#include "Aura/Player/AuraPlayerState.h"
#include "Aura/UI/HUD/AuraHUD.h"
#include "Aura/UI/WidgetController/AuraWidgetController.h"
#include "Kismet/GameplayStatics.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (auto* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) {
		if (auto* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
			auto* PS = PC->GetPlayerState<AAuraPlayerState>();
			auto* ASC = PS->GetAbilitySystemComponent();
			auto* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WCParams(PC, PS, ASC, AS);

			return AuraHUD->GetOverlayWidgetController(WCParams);
		}
	}
	return nullptr;
}