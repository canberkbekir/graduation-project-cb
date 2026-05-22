// RLogSubsystem.cpp
#include "Subsystems/RLogSubsystem.h"
#include "Subsystems/REventBusSubsystem.h"
#include "Core/RCharacterBase.h"

void URLogSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Handler = NewObject<URLogHandlerBase>(this, URLogHandlerBase::StaticClass());

	if (UREventBusSubsystem* EB = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		AbilityResolvedHandle = EB->Subscribe<FAbilityResolved>(this, &URLogSubsystem::OnAbilityResolved);
		CheckResolvedHandle   = EB->Subscribe<FCheckResolved>  (this, &URLogSubsystem::OnCheckResolved);
	}
}

void URLogSubsystem::Deinitialize()
{
	if (UREventBusSubsystem* EB = GetGameInstance()->GetSubsystem<UREventBusSubsystem>())
	{
		EB->Unsubscribe<FAbilityResolved>(AbilityResolvedHandle);
		EB->Unsubscribe<FCheckResolved>  (CheckResolvedHandle);
	}
	Super::Deinitialize();
}

void URLogSubsystem::SetHandlerClass(TSubclassOf<URLogHandlerBase> NewHandlerClass)
{
	UClass* ClassToUse = NewHandlerClass ? NewHandlerClass.Get() : URLogHandlerBase::StaticClass();
	Handler = NewObject<URLogHandlerBase>(this, ClassToUse);
}

void URLogSubsystem::AddEntry(FRLogEntry Entry)
{
	if (!Handler)
	{
		return;
	}

	if (!Handler->ShouldLog(Entry))
	{
		return;
	}

	if (Entry.Timestamp == FDateTime())
	{
		Entry.Timestamp = FDateTime::Now();
	}

	// Let the handler format a message if the caller didn't provide one
	if (Entry.Message.IsEmpty())
	{
		Entry.Message = Handler->FormatEntry(Entry);
	}

	Entries.Add(Entry);
	OnLogEntryAdded.Broadcast(Entry);
	Handler->OnEntryAdded(Entry);
}

void URLogSubsystem::OnAbilityResolved(const FAbilityResolved& Event)
{
	FRLogEntry Entry;
	Entry.Category      = ERLogCategory::Combat;
	auto GetDisplayFName = [](AActor* Actor) -> FName
	{
		if (!Actor) return NAME_None;
		if (const ARCharacterBase* Char = Cast<ARCharacterBase>(Actor))
		{
			const FText Name = Char->GetDisplayName();
			if (!Name.IsEmpty()) return FName(*Name.ToString());
		}
		return FName(*Actor->GetActorNameOrLabel());
	};

	Entry.SourceActorId  = GetDisplayFName(Event.Source.Get());
	Entry.TargetActorId  = GetDisplayFName(Event.Target.Get());
	Entry.bHasDiceRoll   = true;
	Entry.DiceExpression = Event.Expression;
	Entry.DiceResult     = Event.Result;
	Entry.EventTag       = Event.MagnitudeTag;
	AddEntry(Entry);
}

void URLogSubsystem::OnCheckResolved(const FCheckResolved& Event)
{
	auto GetDisplayFName = [](AActor* Actor) -> FName
	{
		if (!Actor) return NAME_None;
		if (const ARCharacterBase* Char = Cast<ARCharacterBase>(Actor))
		{
			const FText Name = Char->GetDisplayName();
			if (!Name.IsEmpty()) return FName(*Name.ToString());
		}
		return FName(*Actor->GetActorNameOrLabel());
	};

	FRLogEntry Entry;
	Entry.Category        = ERLogCategory::Combat;
	Entry.SourceActorId   = GetDisplayFName(Event.Source.Get());
	Entry.TargetActorId   = GetDisplayFName(Event.Target.Get());
	Entry.bHasDiceRoll    = true;
	Entry.DiceExpression  = Event.Expression;
	Entry.DiceResult      = Event.Roll;
	Entry.EventTag        = Event.CheckTag;
	Entry.CheckTargetValue = Event.TargetValue;
	Entry.bCheckSuccess   = Event.bSuccess;
	Entry.bCheckCritical  = Event.bCritical;
	// Message intentionally left empty — FormatEntry in Blueprint handles it.
	AddEntry(Entry);
}
