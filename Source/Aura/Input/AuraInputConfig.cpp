// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraInputConfig.h"

const UInputAction* UAuraInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound) const
{
	for (const auto& Action : AbilityInputActions) {
		if (Action.InputAction && Action.InputTag == InputTag) {
			return Action.InputAction;
		}
	}

	if (bLogNotFound) {
		UE_LOG(LogTemp, Error, TEXT("Can't find AbilityInputAction for InputTag"));
	}

	return nullptr;
}