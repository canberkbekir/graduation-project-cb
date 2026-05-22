#include "Settings/RWorldViewOutlineConfig.h"
#include "GameFramework/Actor.h"

int32 URWorldViewOutlineConfig::GetStencilForActor(const AActor* Actor, EWorldView View) const
{
	if (!Actor)
	{
		return 0;
	}

	for (const FName& Tag : Actor->Tags)
	{
		if (const FOutlineTagEntry* Entry = TagConfig.Find(Tag))
		{
			if (Entry->WorldView == View)
			{
				return Entry->StencilValue;
			}
		}
	}

	return 0;
}

bool URWorldViewOutlineConfig::ActorHasAnyOutlineTag(const AActor* Actor) const
{
	if (!Actor)
	{
		return false;
	}

	for (const FName& Tag : Actor->Tags)
	{
		if (TagConfig.Contains(Tag))
		{
			return true;
		}
	}

	return false;
}
