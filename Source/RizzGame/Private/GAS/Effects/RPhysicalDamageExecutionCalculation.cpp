#include "GAS/Effects/RPhysicalDamageExecutionCalculation.h"

#include "GameplayEffectTypes.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterDebuffAttributeSet.h"

struct FPhysicalDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(KineticShields);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Physique);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageVulnerability);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageFlatBonus);

	FPhysicalDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCombatAttributeSet,
			KineticShields,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCombatAttributeSet,
			Health,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCoreAttributeSet,
			Physique,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterDebuffAttributeSet,
			PhysicalDamageVulnerability,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterDebuffAttributeSet,
			PhysicalDamageFlatBonus,
			Target,
			false);
	}
};

static const FPhysicalDamageStatics& PhysicalDamageStatics()
{
	static FPhysicalDamageStatics Statics;
	return Statics;
}

URPhysicalDamageExecutionCalculation::URPhysicalDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().KineticShieldsDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().PhysiqueDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().PhysicalDamageVulnerabilityDef);
	RelevantAttributesToCapture.Add(PhysicalDamageStatics().PhysicalDamageFlatBonusDef);
}

void URPhysicalDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const float PhysicalDamage =
		Spec.GetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag("Data.Damage.Physical"),
			false, 0.f);

	if (PhysicalDamage <= 0.f)
	{
		return;
	}

	FAggregatorEvaluateParameters Params;

	float Kinetic = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		PhysicalDamageStatics().KineticShieldsDef, Params, Kinetic);

	/* ---------- Physical Resistance Mod (regular attacks) ---------- */
	// floor((Physique - 10) / 2) subtracted from incoming damage before shields.

	float Physique = 10.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		PhysicalDamageStatics().PhysiqueDef, Params, Physique);

	const float ResistMod = static_cast<float>(FMath::FloorToInt((Physique - 10.f) / 2.f));

	float Vulnerability = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		PhysicalDamageStatics().PhysicalDamageVulnerabilityDef, Params, Vulnerability);

	float FlatBonus = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		PhysicalDamageStatics().PhysicalDamageFlatBonusDef, Params, FlatBonus);

	float Remaining = FMath::Max(0.f, PhysicalDamage - ResistMod);
	Remaining *= (1.f + FMath::Max(0.f, Vulnerability));
	Remaining += static_cast<float>(FMath::FloorToInt(FMath::Max(0.f, FlatBonus)));

	if (Remaining <= 0.f)
	{
		return;
	}

	/* ---------- Physical → Kinetic Shields ---------- */

	if (Kinetic > 0.f)
	{
		const float Used = FMath::Min(Kinetic, Remaining);
		Remaining -= Used;

		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				URCharacterCombatAttributeSet::GetKineticShieldsAttribute(),
				EGameplayModOp::Additive,
				-Used));
	}

	/* ---------- Spill to Health ---------- */

	if (Remaining > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				URCharacterCombatAttributeSet::GetHealthAttribute(),
				EGameplayModOp::Additive,
				-Remaining));
	}
}
