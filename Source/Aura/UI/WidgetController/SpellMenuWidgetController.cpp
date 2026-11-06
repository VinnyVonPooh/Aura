// Copyright (c) MarmoDrake. All Rights Reserved.

#include "SpellMenuWidgetController.h"

#include "Aura/AuraGameplayTags.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/Player/AuraPlayerState.h"

void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag) {
		if (AbilityInfo) {
			auto Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	GetAuraPS()->OnSpellPointsChangedDelegate.AddLambda(
		[this](int32 NewSpellPoints) { SpellPointsChangedDelegate.Broadcast(NewSpellPoints); });
}

void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();

	SpellPointsChangedDelegate.Broadcast(GetAuraPS()->GetSpellPoints());
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	const auto GameplayTags = FAuraGameplayTags::Get();
	const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
	FGameplayTag AbilityStatus;

	const auto* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const bool bSpecValid = AbilitySpec != nullptr; // Абилка ещё не открыта и не дана персонажу

	if (!bTagValid || bTagNone || !bSpecValid) {
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	} else {
		AbilityStatus = UAuraAbilitySystemComponent::GetStatusFromSpec(*AbilitySpec);
	}

	bool bShouldEnableSpendPointsButton = false;
	bool bShouldEnableEquipButton = false;
	ShouldEnableButtons(AbilityStatus, SpellPoints, bShouldEnableSpendPointsButton, bShouldEnableEquipButton);
	SpellGlobeSelectedDelegate.Broadcast(bShouldEnableSpendPointsButton, bShouldEnableEquipButton);
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,
													 bool& bShouldEnableSpendPointsButton, bool& bShouldEnableEquipButton)
{
	const auto GameplayTags = FAuraGameplayTags::Get();
	bShouldEnableSpendPointsButton = false;
	bShouldEnableEquipButton = false;

	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped)) {

		bShouldEnableEquipButton = true;
		bShouldEnableSpendPointsButton = SpellPoints > 0;

	} else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible)) {

		bShouldEnableEquipButton = false;
		bShouldEnableSpendPointsButton = SpellPoints > 0;

	} else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked)) {

		bShouldEnableEquipButton = true;
		bShouldEnableSpendPointsButton = SpellPoints > 0;

	} else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Locked)) {

		bShouldEnableEquipButton = false;
		bShouldEnableSpendPointsButton = false;
	}
}
