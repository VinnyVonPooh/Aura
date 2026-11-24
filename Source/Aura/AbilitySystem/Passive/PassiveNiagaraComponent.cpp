// Copyright (c) MarmoDrake. All Rights Reserved.

#include "PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Aura/AbilitySystem/AuraAbilitySystemComponent.h"
#include "Aura/Interaction/CombatInterface.h"

UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());

	if (auto* ASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))) {
		ASC->ActivatePassiveEffect.AddUObject(this, &ThisClass::OnPassiveActivate);
	} else if (CombatInterface) {

		CombatInterface->GetOnOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC) {
			if (auto* ASC = Cast<UAuraAbilitySystemComponent>(InASC)) {
				ASC->ActivatePassiveEffect.AddUObject(this, &ThisClass::OnPassiveActivate);
			}
		});
	}

	if (CombatInterface) {
		CombatInterface->GetOnDeathDelegate().AddDynamic(this, &ThisClass::OnOwnerDeath);
	}
}

void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag)) {
		if (bActivate && !IsActive()) {
			Activate();
		} else {
			Deactivate();
		}
	}
}

void UPassiveNiagaraComponent::OnOwnerDeath(AActor* DeadActor)
{
	Deactivate();
}