#include "UI/CharacterSheet/RCharacterPortraitWidget.h"

#include "Character/RCharacterSpec.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"

void URCharacterPortraitWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetLevel(1);
	SetHealth(0.f, 0.f);
}

void URCharacterPortraitWidget::NativeDestruct()
{
	if (PortraitHandle.IsValid())
	{
		PortraitHandle->CancelHandle();
		PortraitHandle.Reset();
	}

	Super::NativeDestruct();
}

void URCharacterPortraitWidget::InitFromDefinition(const FCharacterDefinitionRow& InDef)
{
	if (Txt_Name)
	{
		Txt_Name->SetText(InDef.DisplayName);
	}

	if (!Img_Portrait)
	{
		return;
	}

	if (InDef.Portrait.IsNull())
	{
		ApplyFallback();
		return;
	}

	CachedPortrait = InDef.Portrait;

	// cancel previous load
	if (PortraitHandle.IsValid())
	{
		PortraitHandle->CancelHandle();
		PortraitHandle.Reset();
	}

	// already loaded?
	if (UTexture2D* Loaded = CachedPortrait.Get())
	{
		Img_Portrait->SetBrushFromTexture(Loaded, true);
		return;
	}

	// async load
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	PortraitHandle = Streamable.RequestAsyncLoad(
		CachedPortrait.ToSoftObjectPath(),
		FStreamableDelegate::CreateUObject(this, &URCharacterPortraitWidget::HandlePortraitLoaded)
	);
}

void URCharacterPortraitWidget::HandlePortraitLoaded()
{
	if (!Img_Portrait)
	{
		return;
	}

	if (UTexture2D* Loaded = CachedPortrait.Get())
	{
		Img_Portrait->SetBrushFromTexture(Loaded, true);
		return;
	}

	ApplyFallback();
	PortraitHandle.Reset();
}

void URCharacterPortraitWidget::SetLevel(int32 InLevel)
{
	if (Txt_Level)
	{
		Txt_Level->SetText(FText::AsNumber(FMath::Max(1, InLevel)));
	}
}

void URCharacterPortraitWidget::SetHealth(float InCurrent, float InMax)
{
	CurrentHP = FMath::Max(0.f, InCurrent);
	MaxHP     = FMath::Max(0.f, InMax);

	const float Pct = (MaxHP > 0.f) ? FMath::Clamp(CurrentHP / MaxHP, 0.f, 1.f) : 0.f;

	if (PB_Health)
	{
		PB_Health->SetPercent(Pct);
	}

	if (Txt_Health)
	{
		const int32 CurInt = FMath::RoundToInt(CurrentHP);
		const int32 MaxInt = FMath::RoundToInt(MaxHP);
		Txt_Health->SetText(FText::FromString(FString::Printf(TEXT("%02d / %02d"), CurInt, MaxInt)));
	}
}

void URCharacterPortraitWidget::SetLevelProgress(float NewCurrentExperience,float MaxExperience)
{
	CurrentExperience = FMath::Max(0.f, NewCurrentExperience);
	ExperienceToLevelUp     = FMath::Max(0.f, MaxExperience);

	const float Pct = (ExperienceToLevelUp > 0.f) ? FMath::Clamp(CurrentExperience / ExperienceToLevelUp, 0.f, 1.f) : 0.f;

	if (PB_Experience)
	{
		PB_Experience->SetPercent(Pct);
	}
}

void URCharacterPortraitWidget::SetEnergyShield(float CurrentShield,float MaxShield)
{
	CurrentEnergyShield = FMath::Max(0.f, CurrentShield);
	MaxEnergyShield     = FMath::Max(0.f, MaxShield);

	const float Pct = (MaxEnergyShield > 0.f) ? FMath::Clamp(CurrentEnergyShield / MaxEnergyShield, 0.f, 1.f) : 0.f;

	if (PB_EnergyShield)
	{
		PB_EnergyShield->SetPercent(Pct);
	}

	if (Txt_EnergyShield)
	{
		const int32 CurInt = FMath::RoundToInt(CurrentEnergyShield);
		const int32 MaxInt = FMath::RoundToInt(MaxEnergyShield);
		Txt_EnergyShield->SetText(FText::FromString(FString::Printf(TEXT("%02d / %02d"), CurInt, MaxInt)));
	}
}
void URCharacterPortraitWidget::SetPhysicalShield(float CurrentShield,float MaxShield)
{
	CurrentPhysicalShield = FMath::Max(0.f, CurrentShield);
	MaxPhysicalShield     = FMath::Max(0.f, MaxShield);

	const float Pct = (MaxPhysicalShield > 0.f) ? FMath::Clamp(CurrentPhysicalShield / MaxPhysicalShield, 0.f, 1.f) : 0.f;

	if (PB_PhysicalShield)
	{
		PB_PhysicalShield->SetPercent(Pct);
	}

	if (Txt_PhysicalShield)
	{
		const int32 CurInt = FMath::RoundToInt(CurrentPhysicalShield);
		const int32 MaxInt = FMath::RoundToInt(MaxPhysicalShield);
		Txt_PhysicalShield->SetText(FText::FromString(FString::Printf(TEXT("%02d / %02d"), CurInt, MaxInt)));
	}
}

void URCharacterPortraitWidget::ApplyFallback()
{
	if (Img_Portrait)
	{
		Img_Portrait->SetBrushFromTexture(nullptr, true);
	}
	if (Txt_Name)
	{
		Txt_Name->SetText(FText::GetEmpty());
	}
}
