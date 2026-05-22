// REventBusSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/NoExportTypes.h"
#include "Templates/Models.h"

#include "REventBusSubsystem.generated.h"

/** How much history to replay when subscribing. */
UENUM()
enum class EReplayMode : uint8
{
	None,
	Last,
	All
};

/**
 * Type-based (USTRUCT) Event Bus.
 * - Each TEvent (USTRUCT) has its own channel.
 * - Publish<T> records to history and broadcasts.
 * - Subscribe<T> lets you listen and optionally replay the past immediately.
 */
UCLASS()
class RIZZGAME_API UREventBusSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/* ───────────────────────────── Publish / Subscribe ───────────────────────────── */

	template <typename TEvent>
	void Publish(const TEvent& E, bool bRecord = true)
	{
		// Enforce: TEvent must be a USTRUCT that provides StaticStruct()
		static_assert(TModels<CStaticStructProvider, TEvent>::Value,
		              "TEvent must be a USTRUCT (must provide StaticStruct()).");

		auto* Ch = GetOrAdd<TEvent>();
		Ch->Publish(E, bRecord);
	}

	template <typename TEvent, class UserClass>
	FDelegateHandle Subscribe(UserClass* Obj, void (UserClass::*Func)(const TEvent&),
	                          EReplayMode ReplayMode = EReplayMode::None)
	{
		static_assert(TModels<CStaticStructProvider, TEvent>::Value,
		              "TEvent must be a USTRUCT (must provide StaticStruct()).");

		check(Obj && Func);

		auto* Ch = GetOrAdd<TEvent>();
		FDelegateHandle Handle = Ch->OnEvent.AddUObject(Obj, Func);

		// Optional immediate replay
		if (ReplayMode != EReplayMode::None && Ch->History.Num() > 0)
		{
			if (ReplayMode == EReplayMode::Last)
			{
				(Obj->*Func)(Ch->History.Last());
			}
			else
			{
				for (const TEvent& Past : Ch->History)
				{
					(Obj->*Func)(Past);
				}
			}
		}
		return Handle;
	}

	/** Unsubscribe a specific handle from this TEvent channel. */
	template <typename TEvent>
	void Unsubscribe(FDelegateHandle Handle)
	{
		if (auto* Ch = GetChannel<TEvent>())
		{
			Ch->OnEvent.Remove(Handle);
		}
	}

	/** Remove Obj from all channels (handy in widget destructors). */
	void UnsubscribeAll(UObject* Obj)
	{
		if (!Obj)
		{
			return;
		}
		for (TPair<const UScriptStruct*, TUniquePtr<FChannelBase>>& Pair : Channels)
		{
			if (Pair.Value)
			{
				Pair.Value->RemoveAll(Obj);
			}
		}
	}

	/* ───────────────────────────── Replay helpers ───────────────────────────── */

	/** Replay: broadcast the last event in this channel to all current listeners. */
	template <typename TEvent>
	bool ReplayLast()
	{
		auto* Ch = GetChannel<TEvent>();
		if (!Ch || Ch->History.Num() == 0)
		{
			return false;
		}
		const TEvent& Last = Ch->History.Last();
		Ch->OnEvent.Broadcast(Last);
		return true;
	}

	/** Replay: broadcast the entire history (in order) to all current listeners. */
	template <typename TEvent>
	int32 ReplayAll()
	{
		auto* Ch = GetChannel<TEvent>();
		if (!Ch)
		{
			return 0;
		}
		int32 Count = 0;
		for (const TEvent& Past : Ch->History)
		{
			Ch->OnEvent.Broadcast(Past);
			++Count;
		}
		return Count;
	}

	/* ───────────────────────────── History management ───────────────────────────── */

	template <typename TEvent>
	void ClearHistory()
	{
		if (auto* Ch = GetChannel<TEvent>())
		{
			Ch->History.Reset();
		}
	}

	template <typename TEvent>
	void SetMaxHistory(int32 NewMax)
	{
		auto* Ch = GetOrAdd<TEvent>();
		Ch->MaxHistory = FMath::Max(0, NewMax);
		if (Ch->MaxHistory > 0 && Ch->History.Num() > Ch->MaxHistory)
		{
			Ch->History.RemoveAt(0, Ch->History.Num() - Ch->MaxHistory);
		}
	}

	template <typename TEvent>
	int32 GetHistoryCount() const
	{
		auto* Ch = GetChannel<TEvent>();
		return Ch ? Ch->History.Num() : 0;
	}

private:
	/* ─────────── Channel types (base + templated) ─────────── */
	struct FChannelBase
	{
		virtual ~FChannelBase()
		{
		}

		virtual void RemoveAll(UObject* Obj) = 0;
	};

	template <typename TEvent>
	struct TChannel : FChannelBase
	{
		DECLARE_MULTICAST_DELEGATE_OneParam(FOnEvent, const TEvent&);
		FOnEvent OnEvent;

		TArray<TEvent> History;
		int32 MaxHistory = 32;

		void Publish(const TEvent& E, bool bRecord)
		{
			if (bRecord && MaxHistory > 0)
			{
				if (History.Num() >= MaxHistory)
				{
					History.RemoveAt(0);
				}
				History.Add(E);
			}
			OnEvent.Broadcast(E);
		}

		virtual void RemoveAll(UObject* Obj) override
		{
			OnEvent.RemoveAll(Obj);
		}
	};

	/* ─────────── Channel access helpers ─────────── */

	// Use StaticStruct() as the map key; avoids TBaseStructure<> pitfalls.
	template <typename TEvent>
	const UScriptStruct* Key() const
	{
		static_assert(TModels<CStaticStructProvider, TEvent>::Value,
		              "TEvent must be a USTRUCT (must provide StaticStruct()).");
		return TEvent::StaticStruct();
	}

	template <typename TEvent>
	TChannel<TEvent>* GetChannel() const
	{
		const UScriptStruct* K = Key<TEvent>();
		if (const TUniquePtr<FChannelBase>* Found = Channels.Find(K))
		{
			return static_cast<TChannel<TEvent>*>(Found->Get());
		}
		return nullptr;
	}

	template <typename TEvent>
	TChannel<TEvent>* GetOrAdd()
	{
		const UScriptStruct* K = Key<TEvent>();
		if (TUniquePtr<FChannelBase>* Found = Channels.Find(K))
		{
			return static_cast<TChannel<TEvent>*>(Found->Get());
		}
		TUniquePtr<TChannel<TEvent>> NewChan = MakeUnique<TChannel<TEvent>>();
		TChannel<TEvent>* Raw = NewChan.Get();
		Channels.Add(K, MoveTemp(NewChan));
		return Raw;
	}

	/** Main map: TEvent::StaticStruct() → Channel */
	TMap<const UScriptStruct*, TUniquePtr<FChannelBase>> Channels;
};
