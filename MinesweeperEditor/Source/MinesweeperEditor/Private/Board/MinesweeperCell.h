#pragma once

#include "CoreMinimal.h"
#include "Types/MinesweeperTypes.h"

/**
 * That represents a single Minesweeper cell
 *
 * Stores whether the cell contains a bomb, the count of adjacent bombs in
 * the 8-neighborhood, and the current reveal state

 * This struct is intentionally lightweight (no UObject/UStruct) and is used by FMinesweeperBoard
 */

struct FMinesweeperCell
{
	bool bHasBomb = false;
	uint8 AdjacentBombs = 0;
	ETileState State = ETileState::Hidden;
	
	/**
	* Restore the cell to its initial “safe & hidden” state
	* Used when starting a new game or clearing the board
	*/
	FORCEINLINE void Reset()
	{
		bHasBomb = false;
		AdjacentBombs = 0;
		State = ETileState::Hidden;
	}
};
