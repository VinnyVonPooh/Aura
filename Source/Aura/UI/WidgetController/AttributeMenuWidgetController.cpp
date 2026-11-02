// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AttributeMenuWidgetController.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AttributeInfo.h"
#include "Aura/Player/AuraPlayerState.h"

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	auto* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	// Привязка изменения аттрибутов и тправка структуры аттрибута в виджет меню аттрибутов
	for (auto& Pair : AS->TagsToAttributes) {
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value())
			.AddLambda([this, Pair](const FOnAttributeChangeData& Data) { BroadcastAttributeInfo(Pair.Key, Pair.Value()); });
	}

	auto* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AuraPlayerState->OnAttributePointsChangedDelegate.AddLambda(
		[this](int32 NewAttributePoints) { AttributePointsChangedDelegate.Broadcast(NewAttributePoints); });
}

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	auto* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
	check(AttributeInfo);

	// Отправка структуры аттрибута в виджет меню аттрибутов
	for (auto& Pair : AS->TagsToAttributes) {
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}

	const auto* AuraPlayerState = CastChecked<AAuraPlayerState>(PlayerState);
	AttributePointsChangedDelegate.Broadcast(AuraPlayerState->GetAttributePoints());
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const
{
	auto Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);

	AttributeInfoDelegate.Broadcast(Info);
}