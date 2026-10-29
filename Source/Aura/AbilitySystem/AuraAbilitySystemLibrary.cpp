// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemLibrary.h"

#include "AbilitySystemComponent.h"
#include "Aura/AuraAbilityTypes.h"
#include "Aura/Game/AuraGameModeBase.h"
#include "Aura/Interaction/CombatInterface.h"
#include "Aura/Player/AuraPlayerState.h"
#include "Aura/UI/HUD/AuraHUD.h"
#include "Aura/UI/WidgetController/AuraWidgetController.h"
#include "Engine/OverlapResult.h"
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
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) {
		return;
	}

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

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC,
													 ECharacterClass CharacterClass)
{
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) {
		return;
	}

	for (const auto AbilityClass : CharacterClassInfo->CommonAbilities) {
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}

	const auto& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (const auto AbilityClass : DefaultInfo.StartupAbilities) {

		if (const auto* CombatInterface = Cast<ICombatInterface>(ASC->GetAvatarActor())) {
			auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, CombatInterface->GetPlayerLevel());
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass,
															 int32 CharacterLevel)
{
	const auto CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) {
		return 0;
	}
	const auto& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = DefaultInfo.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const auto* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (!AuraGameMode) {
		return nullptr;
	}

	return AuraGameMode->CharacterClassInfo;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
		return AuraContext->IsBlockedHit();
	}
	return false;
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const auto* AuraContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
		return AuraContext->IsCriticalHit();
	}
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (auto* AuraContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
		AuraContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (auto* AuraContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get())) {
		AuraContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors,
														   const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	if (const auto* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull)) {

		TArray<FOverlapResult> Overlaps;

		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity,
										FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects),
										FCollisionShape::MakeSphere(Radius), SphereParams);

		for (const auto& Overlap : Overlaps) {

			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor())) {
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

bool UAuraAbilitySystemLibrary::IsNotFriend(const AActor* FirstActor, const AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));

	return !bBothArePlayers && !bBothAreEnemies;
}
