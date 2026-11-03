// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemComponent.h"

#include "Abilities/AuraGameplayAbility.h"
#include "Aura/AuraLogChannels.h"
#include "Aura/Interaction/PlayerInterface.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::ClientEffectApplied);
}

// выполняется только на сервере
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const auto& AbilityClass : StartupAbilities) {
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);

		if (const auto* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability)) {
			AbilitySpec.DynamicAbilityTags.AddTag(AuraAbility->StartupInputTag);
			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast(this);
}

void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const auto& AbilityClass : StartupPassiveAbilities) {
		auto AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) {
		return;
	}

	for (auto& AbilitySpec : GetActivatableAbilities()) {
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {

			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive()) {
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) {
		return;
	}

	for (auto& AbilitySpec : GetActivatableAbilities()) {
		if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag)) {

			AbilitySpecInputReleased(AbilitySpec);
		}
	}
}

void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	// блокируем систему, чтобы она не зменилась до конца функции - сигнал может ловиться там, где система может меняться, что приведёт к
	// неопределённому результату
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (auto& AbilitySpec : GetActivatableAbilities()) {
		if (!Delegate.ExecuteIfBound(AbilitySpec)) {
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability) {

		for (const auto& Tag : AbilitySpec.Ability.Get()->AbilityTags) {
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities")))) {
				return Tag;
			}
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability) {

		for (const auto& Tag : AbilitySpec.DynamicAbilityTags) {
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag")))) {
				return Tag;
			}
		}
	}

	return FGameplayTag();
}

void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>()) {
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0) {
			
			// Вызов функции на сервере
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	// слушаем через абилку GA_ListenForEvents
	// а потом применяем через GE_EventBasedEffect
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>()) {
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	//  для клиента
	if (!bStartupAbilitiesGiven) {
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast(this);
	}
}

// Для того чтобы эффект ловился и на клиенте и на сервере необходимо использовать такой нейминг конвеншн Client..._Implementation
void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
																	 const FGameplayEffectSpec& EffectSpec,
																	 FActiveGameplayEffectHandle ActiveEffectHandle) const
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}