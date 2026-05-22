// RFogGrid.cpp
#include "FogOfWar/RFogGrid.h"

void FRFogGrid::Initialize(FVector2D InOrigin, float InCellSize, int32 InWidth, int32 InHeight)
{
	WorldOrigin = InOrigin;
	CellSize    = InCellSize;
	GridWidth   = InWidth;
	GridHeight  = InHeight;
	Cells.Init(static_cast<uint8>(ERFogState::Unknown), GridWidth * GridHeight);
}

FIntPoint FRFogGrid::WorldToCell(const FVector& WorldPos) const
{
	return FIntPoint(
		FMath::FloorToInt((WorldPos.X - WorldOrigin.X) / CellSize),
		FMath::FloorToInt((WorldPos.Y - WorldOrigin.Y) / CellSize)
	);
}

FVector2D FRFogGrid::CellToWorld(int32 X, int32 Y) const
{
	return FVector2D(
		WorldOrigin.X + (X + 0.5f) * CellSize,
		WorldOrigin.Y + (Y + 0.5f) * CellSize
	);
}

bool FRFogGrid::IsValidCell(int32 X, int32 Y) const
{
	return X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight;
}

ERFogState FRFogGrid::GetState(int32 X, int32 Y) const
{
	if (!IsValidCell(X, Y))
	{
		return ERFogState::Unknown;
	}
	return static_cast<ERFogState>(Cells[Y * GridWidth + X]);
}

void FRFogGrid::SetState(int32 X, int32 Y, ERFogState State)
{
	if (IsValidCell(X, Y))
	{
		Cells[Y * GridWidth + X] = static_cast<uint8>(State);
	}
}

void FRFogGrid::DemoteAllVisible()
{
	for (uint8& Cell : Cells)
	{
		if (static_cast<ERFogState>(Cell) == ERFogState::Visible)
		{
			Cell = static_cast<uint8>(ERFogState::Explored);
		}
	}
}

void FRFogGrid::MarkCellVisible(int32 X, int32 Y)
{
	if (IsValidCell(X, Y))
	{
		Cells[Y * GridWidth + X] = static_cast<uint8>(ERFogState::Visible);
	}
}
