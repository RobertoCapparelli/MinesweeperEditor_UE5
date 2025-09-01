#include "Board/MinesweeperBoard.h"

#include "Algo/RandomShuffle.h"
#include "Types/MinesweeperTypes.h"

void FMinesweeperBoard::StartNewGame(const FMinesweeperConfig& InConfig)
{
	//Validate and clamp all parameters before mutating the board state
	FMinesweeperConfig TempConfig = InConfig;
	TempConfig.Width = FMath::Clamp(TempConfig.Width, Limits::MinWidth, Limits::MaxWidth);
	TempConfig.Height = FMath::Clamp(TempConfig.Height, Limits::MinHeight, Limits::MaxHeight);
	TempConfig.Bombs = FMath::Clamp(TempConfig.Bombs, Limits::MinBombs,
	                                Limits::MaxBombsFor(TempConfig.Width, TempConfig.Height));
	//Change local config
	Config = TempConfig;
	Width = TempConfig.Width;
	Height = TempConfig.Height;

	//Reset board to initial status
	Cells.SetNum(Width * Height);
	for (FMinesweeperCell& Cell : Cells)
	{
		Cell.Reset();
	}
	bGameOver = false;
	bWin = false;
	RevealedSafeCells = 0;
	bFirstMoveDone = false;

	//Set bombs and compute Adjacency
	PlaceBombs();
	ComputeAdjacency();
}

void FMinesweeperBoard::PlaceBombs()
{
	const int32 TotalCells = Width * Height;

	//reserve and initialize the array 
	TArray<int32> Indices;
	Indices.Reserve(TotalCells);
	for (int32 Index = 0; Index < TotalCells; ++Index)
	{
		Indices.Add(Index);
	}
	//Random without seed (Not specified required in the track)
	Algo::RandomShuffle(Indices);

	for (int32 Index = 0; Index < Config.Bombs; ++Index)
	{
		const int32 CurrentCell = Indices[Index];

		// Convert index in coords x,y
		const int32 X = CurrentCell % Width;
		const int32 Y = CurrentCell / Width;

		//Current cell has the bomb
		FMinesweeperCell& Cell = At(X, Y);
		Cell.bHasBomb = true;
	}
}
//Relocate bombs to avoid first click with bomb
void FMinesweeperBoard::RelocateBombFrom(int32 X, int32 Y)
{
	//Check if the cell has bomb
	FMinesweeperCell& Hit = At(X, Y);
	if (!Hit.bHasBomb)
	{
		return;
	}
	Hit.bHasBomb = false;

	// Check for safe cell and relocate
	for (int32 y = 0; y < Height; ++y)
	{
		for (int32 x = 0; x < Width; ++x)
		{
			if (x == X && y == Y)
			{
				continue;
			}
			
			FMinesweeperCell& CurrentCell = At(x, y);
			if (!CurrentCell.bHasBomb)
			{
				CurrentCell.bHasBomb = true;
				ComputeAdjacency();
				return;
			}
		}
	}
}
FMinesweeperBoard::ERevealOutcome FMinesweeperBoard::Reveal(int32 X, int32 Y)
{
	if (bGameOver || bWin)
	{
		return ERevealOutcome::None;
	}

	if (!IsValid(X, Y))
	{
		return ERevealOutcome::None;
	}

	//Get the cell clicked
	FMinesweeperCell& Cell = At(X, Y);

	//First-Move safe
	if (!bFirstMoveDone)
	{
		bFirstMoveDone = true;
		if (Cell.bHasBomb)
		{
			RelocateBombFrom(X, Y);
		}
	}
	
	//Already clicked, return
	if (Cell.State == ETileState::Revealed || Cell.State == ETileState::Exploded)
	{
		return ERevealOutcome::AlreadyRevealed;
	}
	
	//If cell has bomb set game over
	if (Cell.bHasBomb)
	{
		Cell.State = ETileState::Exploded;
		bGameOver = true;
		return ERevealOutcome::Exploded;
	}

	//Show cell
	Cell.State = ETileState::Revealed;
	++RevealedSafeCells;

	//if cell don't have adjacent bombs show adjacent cell
	if (Cell.AdjacentBombs == 0)
	{
		FloodReveal(X, Y);
	}

	//CHECK WIN - if the total cell safe are shown set bWin
	const int32 TotalSafe = Width * Height - Config.Bombs;
	if (RevealedSafeCells >= TotalSafe)
		bWin = true;

	return ERevealOutcome::Revealed;
}

//Compute AdjacentBombs for every non-bomb cell
void FMinesweeperBoard::ComputeAdjacency()
{
	for (int32 YIndex = 0; YIndex < Height; ++YIndex)
	{
		for (int32 XIndex = 0; XIndex < Width; ++XIndex)
		{
			FMinesweeperCell& Cell = At(XIndex, YIndex);
			//If current cell has bomb avoid 
			if (Cell.bHasBomb)
			{
				Cell.AdjacentBombs = 0;
				continue;
			}

			uint8 Count = 0;

			//Count bombs in the 8-neighborhood
			ForEachNeighbor(XIndex, YIndex, [this, &Count](int32 CurrentAdjX, int32 CurrentAdjY)
			{
				if (At(CurrentAdjX, CurrentAdjY).bHasBomb)
				{
					++Count;
				}
			});

			Cell.AdjacentBombs = Count;
		}
	}
}

/*BFS for reval from a zero-adjacency cell
*Reveals safe neighbors
*/
void FMinesweeperBoard::FloodReveal(int32 X, int32 Y)
{
	if (!ensureMsgf(IsValid(X,Y), TEXT("FloodReveal called with out-of-bounds coords (%d,%d)"), X, Y))
	{
		return;
	}
	if (!ensureMsgf(!At(X,Y).bHasBomb && At(X,Y).AdjacentBombs == 0, TEXT("FloodReveal should start from a safe zero-adjacency cell (%d,%d)"), X, Y))
	{
		return;
	}

	TQueue<FCellCoord> Queue;
	Queue.Enqueue(FCellCoord(X, Y));

	while (!Queue.IsEmpty())
	{
		FCellCoord CurrentCell;
		Queue.Dequeue(CurrentCell);

		ForEachNeighbor(CurrentCell.X, CurrentCell.Y, [this, &Queue](int32 CurrentAdjX, int32 CurrentAdjY)
		{
			TryRevealSafeCell(CurrentAdjX, CurrentAdjY, Queue);
		});
	}
}

/*
* Reveals a safe Hidden cell at (X, Y) and, if it is a zero-adjacency cell,
* enqueues it to expand the BFS flood
*/
void FMinesweeperBoard::TryRevealSafeCell(int32 X, int32 Y, TQueue<FCellCoord>& Queue)
{
	FMinesweeperCell& CurrentCell = At(X, Y);

	// Skip bombs and already processed cells
	if (CurrentCell.bHasBomb || CurrentCell.State != ETileState::Hidden)
	{
		return;
	}

	// Reveal the safe cell and update counter
	CurrentCell.State = ETileState::Revealed;
	++RevealedSafeCells;

	// If it has 0 adj bombs, push it to the BFS frontier so neighbors will be explored
	if (CurrentCell.AdjacentBombs == 0)
	{
		Queue.Enqueue(FCellCoord(X, Y));
	}
}
