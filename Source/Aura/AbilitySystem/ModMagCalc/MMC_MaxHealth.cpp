// Copyright (c) MarmoDrake. All Rights Reserved.

#include "MMC_MaxHealth.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"

UMMC_MaxHealth::UMMC_MaxHealth()
{
	// Задаём аттрибут от которого зависим
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// ----------- Получение значения Vigor ---------------------
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	// --------------- Получение значения уровня игрока -----------------
	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>()) {
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// --------------- Рассчёт финального значения аттрибута MaxHealth -----------------
	return 80.f + 2.5f * Vigor + 10.f * PlayerLevel;
}