// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AttributeMenuWidgetController.h"

#include "Aura/AuraGameplayTags.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AttributeInfo.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies() {}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	auto* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	// Отправка структуры аттрибута в виджет меню аттрибутов
	for (auto& Pair : AS->TagsToAttributes) {
		auto Info = AttributeInfo->FindAttributeInfoForTag(Pair.Key);
		Info.AttributeValue = Pair.Value().GetNumericValue(AS);

		AttributeInfoDelegate.Broadcast(Info);
	}
}