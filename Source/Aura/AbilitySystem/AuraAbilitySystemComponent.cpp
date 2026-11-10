// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemComponent.h"

#include "AuraAbilitySystemLibrary.h"
#include "Abilities/AuraGameplayAbility.h"
#include "Aura/AuraGameplayTags.h"
#include "Aura/AuraLogChannels.h"
#include "Aura/Interaction/PlayerInterface.h"
#include "Data/AbilityInfo.h"

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
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
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

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability) {

		for (const auto& StatusTag : AbilitySpec.DynamicAbilityTags) {
			if (StatusTag.MatchesTag(FAuraGameplayTags::Get().Abilities_Status)) {
				return StatusTag;
			}
		}
	}

	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const auto* Spec = GetSpecFromAbilityTag(AbilityTag)) {
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const auto* Spec = GetSpecFromAbilityTag(AbilityTag)) {
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	// Локаем лист абилок, чтобы не поменять в цикле
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (auto& AbilitySpec : GetActivatableAbilities()) {
		for (const auto& Tag : AbilitySpec.Ability.Get()->AbilityTags) {
			if (Tag.MatchesTag(AbilityTag)) {
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
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

// предоставление абилок (спеллов) которые подходят по уровню
// Выполняется на сервере
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	auto* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (const auto& Info : AbilityInfo->AbilityInformation) {

		if (!Info.AbilityTag.IsValid()) {
			continue;
		}
		if (Level > Info.LevelRequirement) {
			continue;
		}
		// проверка, что абилка не была уже предоставлена персонажу
		if (!GetSpecFromAbilityTag(Info.AbilityTag)) {
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.DynamicAbilityTags.AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);

			MarkAbilitySpecDirty(AbilitySpec);

			// отправка сигнала и на сервер и на клиент
			ClientUpdateAbilityStatus(Info.AbilityTag, FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

// Функция для эквипа спелла, назначение новых кнопок
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (auto* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {
		const FGameplayTag PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);

		const auto& GameplayTags = FAuraGameplayTags::Get();

		if (Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked) {
			// Убираем установленный инпут тег из всех абилок (назначение кнопок)
			ClearAbilitiesOfSlot(Slot);

			// Чистим инпут у изменяемой абилки
			ClearSlot(*AbilitySpec);

			// Назначаем абилку на новый инпут
			AbilitySpec->DynamicAbilityTags.AddTag(Slot);

			// ставим абилке статус экипированной
			if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked)) {
				AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			MarkAbilitySpecDirty(*AbilitySpec);
		}

		ClientEquipAbility_Implementation(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status,
	const FGameplayTag& Slot, const FGameplayTag& PrevSlot)
{
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PrevSlot);
}

bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
                                                              FString& OutNextLevelDescription)
{
	if (const auto* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {

		if (const auto* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability)) {
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	const auto* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None)) {
		OutDescription = FString();
	} else {
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}

	OutNextLevelDescription = FString();
	return false;
}

void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec& Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(Spec);
	Spec.DynamicAbilityTags.RemoveTag(Slot);
	MarkAbilitySpecDirty(Spec);
}

void UAuraAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (auto& Spec : GetActivatableAbilities()) {
		if (AbilityHasSlot(Spec, Slot)) {
			ClearSlot(Spec);
		}
	}
}

bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	for (auto Tag : Spec.DynamicAbilityTags) {
		if (Tag.MatchesTagExact(Slot)) {
			return true;
		}
	}
	return false;
}

void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (auto* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) {

		// Отнимаем от очков спелла
		if (GetAvatarActor()->Implements<UPlayerInterface>()) {
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}

		// начисляем очко в выбранный спелл
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		const auto GameplayTags = FAuraGameplayTags::Get();

		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible)) {

			AbilitySpec->DynamicAbilityTags.RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->DynamicAbilityTags.AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;

		} else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) ||
				   Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked)) {

			AbilitySpec->Level += 1;
		}

		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag,
																		   int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

void UAuraAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();

	//  для клиента
	if (!bStartupAbilitiesGiven) {
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
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