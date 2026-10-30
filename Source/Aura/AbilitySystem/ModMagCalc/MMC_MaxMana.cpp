// Copyright (c) MarmoDrake. All Rights Reserved.

#include "MMC_MaxMana.h"

#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/Interaction/CombatInterface.h"

UMMC_MaxMana::UMMC_MaxMana()
{ 
	// Задаём аттрибут от которого зависим
	IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntelligenceDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// ----------- Получение значения Intelligence ---------------------
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float Intelligence = 0.f;
	GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
	Intelligence = FMath::Max<float>(Intelligence, 0.f);

	// --------------- Получение значения уровня игрока -----------------

	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>()) {
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// --------------- Рассчёт финального значения аттрибута MaxMana -----------------
	return 50.f + 2.5f * Intelligence + 15.f * PlayerLevel;
}