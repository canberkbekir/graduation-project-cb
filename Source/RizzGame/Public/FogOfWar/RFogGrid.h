// RFogGrid.h
#pragma once

#include "CoreMinimal.h"
#include "RFogGrid.generated.h"

UENUM(BlueprintType)
enum class ERFogState : uint8
{
	Unknown  UMETA(DisplayName = "Unknown"),
	Explored UMETA(DisplayName = "Explored"),
	Visible  UMETA(DisplayName = "Visible"),
};

USTRUCT(BlueprintType)
struct RIZZGAME_API FRFogGrid
{
	GENERATED_BODY()

	UPROPERTY()
	int32 GridWidth = 0;

	UPROPERTY()
	int32 GridHeight = 0;

	UPROPERTY()
	FVector2D WorldOrigin = FVector2D::ZeroVector;

	UPROPERTY()
	float CellSize = 100.0f;

	UPROPERTY()
	TArray<uint8> Cells;

	void Initialize(FVector2D InOrigin, float InCellSize, int32 InWidth, int32 InHeight);

	FIntPoint WorldToCell(const FVector& WorldPos) const;
	FVector2D CellToWorld(int32 X, int32 Y) const;
	bool IsValidCell(int32 X, int32 Y) const;

	ERFogState GetState(int32 X, int32 Y) const;
	void SetState(int32 X, int32 Y, ERFogState State);

	void DemoteAllVisible();
	void MarkCellVisible(int32 X, int32 Y);
};
