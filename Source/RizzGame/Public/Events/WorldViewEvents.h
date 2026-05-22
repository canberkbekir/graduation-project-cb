// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WorldViewEvents.generated.h"

UENUM(BlueprintType)
enum class EWorldView : uint8
{
	Physical,
	Network
};

UENUM(BlueprintType)
enum class EWorldViewPhase : uint8
{
	Idle,
	PreToggle,
	Applying,
	PostToggle
};

/** Published when the full toggle cycle completes. Also published during Applying phase for backward compatibility. */
USTRUCT(BlueprintType)
struct FWorldViewChanged
{
	GENERATED_BODY()

	UPROPERTY()
	EWorldView NewView = EWorldView::Physical;
};

/** Published when a toggle begins — before the PostProcessVolume is flipped. */
USTRUCT(BlueprintType)
struct FWorldViewTogglePreBegin
{
	GENERATED_BODY()

	UPROPERTY()
	EWorldView FromView = EWorldView::Physical;

	UPROPERTY()
	EWorldView ToView = EWorldView::Network;
};

/** Published immediately after the PostProcessVolume is flipped. */
USTRUCT(BlueprintType)
struct FWorldViewToggleApplied
{
	GENERATED_BODY()

	UPROPERTY()
	EWorldView NewView = EWorldView::Physical;
};

/** Published when the post-toggle phase begins. */
USTRUCT(BlueprintType)
struct FWorldViewTogglePostBegin
{
	GENERATED_BODY()

	UPROPERTY()
	EWorldView NewView = EWorldView::Physical;
};

/** Published when a pending toggle is cancelled before the PostProcessVolume is flipped. */
USTRUCT(BlueprintType)
struct FWorldViewToggleCancelled
{
	GENERATED_BODY()

	UPROPERTY()
	EWorldView AbortedAtView = EWorldView::Physical;
};
