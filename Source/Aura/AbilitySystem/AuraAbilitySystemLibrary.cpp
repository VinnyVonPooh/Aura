// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "Aura/Game/AuraGameModeBase.h"
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

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	if (auto* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0)) {
		if (auto* AuraHUD = Cast<AAuraHUD>(PC->GetHUD())) {
			auto* PS = PC->GetPlayerState<AAuraPlayerState>();
			auto* ASC = PS->GetAbilitySystemComponent();
			auto* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WCParams(PC, PS, ASC, AS);

			return AuraHUD->GetAttributeMenuWidgetController(WCParams);
		}
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level,
															UAbilitySystemComponent* ASC)
{
	const auto* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!AuraGameMode) {
		return;
	}

	const auto CharacterClassInfo = AuraGameMode->CharacterClassInfo;
	const auto ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	//------------------------------------

	auto PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(ASC->GetAvatarActor());

	const auto PrimaryAttributesSpecHandle =
		ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	//------------------------------------

	auto SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(ASC->GetAvatarActor());

	const auto SecondaryAttributesSpecHandle =
		ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	//------------------------------------

	auto VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(ASC->GetAvatarActor());

	const auto VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC)
{
	const auto* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!AuraGameMode) {
		return;
	}

	const auto CharacterClassInfo = AuraGameMode->CharacterClassInfo;

	for (const auto AbilityClass : CharacterClassInfo->CommonAbilities) {
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
}