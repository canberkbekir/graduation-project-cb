#include "GAS/Effects/RNetworkDamageExecutionCalculation.h"

#include "GameplayEffectTypes.h"
#include "GAS/Attributes/RCharacterCombatAttributeSet.h"
#include "GAS/Attributes/RCharacterCoreAttributeSet.h"
#include "GAS/Attributes/RCharacterDebuffAttributeSet.h"

struct FNetworkDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(EnergyShields);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Tech);
	DECLARE_ATTRIBUTE_CAPTUREDEF(NetworkDamageVulnerability);
	DECLARE_ATTRIBUTE_CAPTUREDEF(NetworkDamageFlatBonus);

	FNetworkDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCombatAttributeSet,
			EnergyShields,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCombatAttributeSet,
			Health,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterCoreAttributeSet,
			Tech,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterDebuffAttributeSet,
			NetworkDamageVulnerability,
			Target,
			false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(
			URCharacterDebuffAttributeSet,
			NetworkDamageFlatBonus,
			Target,
			false);
	}
};

static const FNetworkDamageStatics& NetworkDamageStatics()
{
	static FNetworkDamageStatics Statics;
	return Statics;
}

URNetworkDamageExecutionCalculation::URNetworkDamageExecutionCalculation()
{
	RelevantAttributesToCapture.Add(NetworkDamageStatics().EnergyShieldsDef);
	RelevantAttributesToCapture.Add(NetworkDamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(NetworkDamageStatics().TechDef);
	RelevantAttributesToCapture.Add(NetworkDamageStatics().NetworkDamageVulnerabilityDef);
	RelevantAttributesToCapture.Add(NetworkDamageStatics().NetworkDamageFlatBonusDef);
}

void URNetworkDamageExecutionCalculation::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const float NetworkDamage =
		Spec.GetSetByCallerMagnitude(
			FGameplayTag::RequestGameplayTag("Data.Damage.Network"),
			false, 0.f);

	if (NetworkDamage <= 0.f)
	{
		return;
	}

	FAggregatorEvaluateParameters Params;

	float Energy = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		NetworkDamageStatics().EnergyShieldsDef, Params, Energy);

	/* ---------- Network Firewall Mod (regular attacks) ---------- */
	// floor((Tech - 10) / 2) subtracted from incoming damage before shields.

	float Tech = 10.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		NetworkDamageStatics().TechDef, Params, Tech);

	const float FirewallMod = static_cast<float>(FMath::FloorToInt((Tech - 10.f) / 2.f));

	float Vulnerability = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		NetworkDamageStatics().NetworkDamageVulnerabilityDef, Params, Vulnerability);

	float FlatBonus = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		NetworkDamageStatics().NetworkDamageFlatBonusDef, Params, FlatBonus);

	float Remaining = FMath::Max(0.f, NetworkDamage - FirewallMod);
	Remaining *= (1.f + FMath::Max(0.f, Vulnerability));
	Remaining += static_cast<float>(FMath::FloorToInt(FMath::Max(0.f, FlatBonus)));

	if (Remaining <= 0.f)
	{
		return;
	}

	/* ---------- Network → Energy Shields ---------- */

	if (Energy > 0.f)
	{
		const float Used = FMath::Min(Energy, Remaining);
		Remaining -= Used;

		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				URCharacterCombatAttributeSet::GetEnergyShieldsAttribute(),
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
