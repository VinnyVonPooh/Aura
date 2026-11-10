// Copyright (c) MarmoDrake. All Rights Reserved.

#include "OverlayWidgetController.h"

#include "Aura/AuraGameplayTags.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/AbilityInfo.h"
#include "Aura/AbilitySystem/Data/LevelUpInfo.h"
#include "Aura/Player/AuraPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	const auto* AuraAS = GetAuraAS();
	OnHealthChanged.Broadcast(AuraAS->GetHealth());
	OnMaxHealthChanged.Broadcast(AuraAS->GetMaxHealth());
	OnManaChanged.Broadcast(AuraAS->GetMana());
	OnMaxManaChanged.Broadcast(AuraAS->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &ThisClass::OnXpChanged);

	GetAuraPS()->OnLevelChangedDelegate.AddLambda([this](int32 NewLevel) { OnPlayerLevelChangedDelegate.Broadcast(NewLevel); });

	const auto* AuraAS = GetAuraAS();

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data) { OnHealthChanged.Broadcast(Data.NewValue); });

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxHealthAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data) { OnMaxHealthChanged.Broadcast(Data.NewValue); });

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetManaAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data) { OnManaChanged.Broadcast(Data.NewValue); });

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAS->GetMaxManaAttribute())
		.AddLambda([this](const FOnAttributeChangeData& Data) { OnMaxManaChanged.Broadcast(Data.NewValue); });

	if (auto* AuraASC = GetAuraASC()) {

		AuraASC->AbilityEquipped.AddUObject(this, &ThisClass::OnAbilityEquipped);

		// если абилки уже даны персонажу, то просто вызываем функцию, если нет, то ждём, когда они будут даны
		if (AuraASC->bStartupAbilitiesGiven) {
			BroadcastAbilityInfo();
		} else {
			AuraASC->AbilitiesGivenDelegate.AddUObject(this, &UAuraWidgetController::BroadcastAbilityInfo);
		}

		AuraASC->EffectAssetTags.AddLambda([this](const FGameplayTagContainer& AssetTags) {
			for (const auto& Tag : AssetTags) {

				if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Message")))) {
					const auto* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
					MessageWidgetRow.Broadcast(*Row);
				}
			}
		});
	}
}

void UOverlayWidgetController::OnXpChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	check(LevelUpInfo);

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0) {
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelUpRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelUpRequirement);
		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot,
												 const FGameplayTag& PrevSlot) const
{
	const auto GameplayTags = FAuraGameplayTags::Get();

	// если спелл раньше стоял на каком-то слоте, то чистим инфу на нём
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// Отправляем информацию о новом выставленном спелле
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
