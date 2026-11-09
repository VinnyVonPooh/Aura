// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraGameplayAbility.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"

FString UAuraGameplayAbility::GetDescription(int32 Level) const
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"),
						   L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum", Level);
}

FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level) const
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n <Default>Causes much more damage  </>"), Level);
}

FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked until Level: </><Level>%d</>"), Level);
}

float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	if (const auto* CostEffect = GetCostGameplayEffect()) {
		for (const auto Mod : CostEffect->Modifiers) {
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute()) {
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const auto* CooldownEffect = GetCooldownGameplayEffect()) {
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}