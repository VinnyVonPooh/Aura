// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraAbilitySystemGlobals.h"

#include "Aura/AuraAbilityTypes.h"

FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAuraGameplayEffectContext();
}