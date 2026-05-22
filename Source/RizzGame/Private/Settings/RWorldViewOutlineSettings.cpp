#include "Settings/RWorldViewOutlineSettings.h"

URWorldViewOutlineSettings::URWorldViewOutlineSettings()
{
	CategoryName = TEXT("Game");
}

URWorldViewOutlineConfig* URWorldViewOutlineSettings::LoadConfig()
{
	const URWorldViewOutlineSettings* Settings = GetDefault<URWorldViewOutlineSettings>();
	if (Settings && !Settings->OutlineConfig.IsNull())
	{
		return Settings->OutlineConfig.LoadSynchronous();
	}
	return nullptr;
}
