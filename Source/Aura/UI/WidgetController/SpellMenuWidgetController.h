// Copyright (c) MarmoDrake. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AuraWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

struct FGameplayTag;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled);

UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()

public:
	virtual void BindCallbacksToDependencies() override;
	virtual void BroadcastInitialValues() override;

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UPROPERTY(BlueprintAssignable, Category = "GAS|Spell")
	FOnPlayerStatChangedSignature SpellPointsChangedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Spell")
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

private:
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpendPointsButton,
									bool& bShouldEnableEquipButton);
};
