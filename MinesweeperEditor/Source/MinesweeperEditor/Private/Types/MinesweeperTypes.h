#pragma once
#include "CoreMinimal.h"

/**
 * Game configuration for a Minesweeper round
 * Width/Height define the grid size, Bombs is the number of mines to place
 * Values are validated/clamped elsewhere against project limits
 */
struct FMinesweeperConfig
{
	int32 Width = 10;
	int32 Height = 10;
	int32 Bombs = 10;
};

//Logical state for a board cell
enum ETileState : uint8
{
	Hidden,
	Revealed,
	Exploded
};

//2D coordinates for cell positions
using FCellCoord = FIntPoint;

//Convert (X,Y) coordinates to linear array index
inline int32 ToIndex(const FCellCoord& CellCord, int32 Width)
{
	return CellCord.Y * Width + CellCord.X;
}

 //Centralized input limits used by both UI and Model 
namespace Limits
{
	inline constexpr int32 MinWidth  = 3;
	inline constexpr int32 MaxWidth  = 100;
	inline constexpr int32 MinHeight = 3;
	inline constexpr int32 MaxHeight = 100;
	inline constexpr int32 MinBombs  = 1;

	constexpr int32 MaxBombsFor(int32 Width, int32 Height)
	{
		const int32 Total = Width * Height;
		return Total > MinBombs ? Total - 1 : MinBombs;
	}
}