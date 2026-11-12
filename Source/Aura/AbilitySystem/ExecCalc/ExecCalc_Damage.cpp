// Copyright (c) MarmoDrake. All Rights Reserved.

#include "ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "Aura/AuraAbilityTypes.h"
#include "Aura/AuraGameplayTags.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/AbilitySystem/AuraAttributeSet.h"
#include "Aura/AbilitySystem/Data/CharacterClassInfo.h"
#include "Aura/Interaction/CombatInterface.h"

struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);

	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);
	}
};

static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);

	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	const auto& GameplayTags = FAuraGameplayTags::Get();
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);

	TagsToCaptureDefs.Add(GameplayTags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
	TagsToCaptureDefs.Add(GameplayTags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	

	const auto* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const auto* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	const AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	const AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>()) {
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>()) {
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const auto& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// ----------- Дебафф ------------

	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters, TagsToCaptureDefs);

	// ----------- Получение значения дамага из функционала Set by Caller ------------
	float Damage = 0;
	// Объединение дамагов разного типа (огонь и др)
	// Применение сопротивлений к дамагу
	for (const auto& DamageTagPair : GameplayTags.DamageTypesToResistances) {
		const FGameplayTag DamageTypeTag = DamageTagPair.Key;
		const FGameplayTag ResistanceTag = DamageTagPair.Value;

		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);

		check(TagsToCaptureDefs.Contains(ResistanceTag));
		const auto CaptureDef = TagsToCaptureDefs[ResistanceTag];
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
		
		DamageTypeValue *= (100.f - Resistance) / 100.f;
		Damage += DamageTypeValue;
	}

	// ----------- Использование аттрибута BlockChance у таргета. Если успешно заблокировано, то дамаг уменьшается вдвое ------------
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(0.f, TargetBlockChance);

	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	Damage = bBlocked ? Damage * 0.5f : Damage;

	// ----------- Получение аттрибута Armor у таргета ------------
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(0.f, TargetArmor);

	// ----------- Получение аттрибута ArmorPenetration у источника ------------
	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters,
															   SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(0.f, SourceArmorPenetration);

	// ----------- Применение аттрибутов Armor и ArmorPenetration ------------
	const auto* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve =
		CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);

	const FRealCurve* EffectiveArmorCurve =
		CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);

	const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;

	// ----------- Получение аттрибутов CriticalHit ------------

	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters,
															   SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(0.f, SourceCriticalHitChance);

	float SourceCriticalHitDamage = 0.f;

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters,
															   SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(0.f, SourceCriticalHitDamage);

	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters,
															   TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(0.f, TargetCriticalHitResistance);

	// ----------- Применение аттрибутов CriticalHit ------------

	const FRealCurve* CriticalHitResistanceCurve =
		CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;

	// ----------- Установка в контекст значений bBlocked и bCriticalHit ---------------------------

	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
	UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);
	UAuraAbilitySystemLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}


void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec,
									   const FAggregatorEvaluateParameters& EvaluationParameters,
									   const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToCaptureDefs) const
{

	const auto& GameplayTags = FAuraGameplayTags::Get();
	for (const auto& Pair : GameplayTags.DamageTypesToDebuffs) {

		const FGameplayTag DamageType = Pair.Key;
		const FGameplayTag DebuffType = Pair.Value;

		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1);
		if (TypeDamage > 0) {
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1);

			// вычисляем сопротивление к дебафу
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
			const auto CaptureDef = InTagsToCaptureDefs[ResistanceTag];

			float TargetDebuffResistance = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Clamp(TargetDebuffResistance, 0.f, 100.f);

			// Уменьшаем шанс дебафа в зависимости от сопротивления
			const float EffectiveDebuffChance = SourceDebuffChance * (100 - TargetDebuffResistance) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff) {
				FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

				const float SourceDebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);
				const float SourceDebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);
				const float SourceDebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);

				UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(EffectContextHandle, true);
				UAuraAbilitySystemLibrary::SetDebuffDamage(EffectContextHandle, SourceDebuffDamage);
				UAuraAbilitySystemLibrary::SetDebuffFrequency(EffectContextHandle, SourceDebuffFrequency);
				UAuraAbilitySystemLibrary::SetDebuffDuration(EffectContextHandle, SourceDebuffDuration);
				UAuraAbilitySystemLibrary::SetDamageType(EffectContextHandle, DamageType);
			}
		}
	}
}