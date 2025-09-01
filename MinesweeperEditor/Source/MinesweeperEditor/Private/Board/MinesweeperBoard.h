#pragma once
#include "CoreMinimal.h"
#include "Types/MinesweeperTypes.h"
#include "Board/MinesweeperCell.h"
#include "Containers/Queue.h"


/*
 * Pure game logic for Minesweeper (no rendering, no UObject)
 *
 * Responsibilities:
 * Initialize a new game, track game state
 * Handle reval rules (single cell, flood-fill)
 */

class FMinesweeperBoard
{
public:
    // Result of a reveal operation
    enum class ERevealOutcome : uint8
    {
        None,
        AlreadyRevealed,
        Revealed,
        Exploded
    };

    //Game API
    void StartNewGame(const FMinesweeperConfig& InConfig);
    
    ERevealOutcome Reveal(int32 X, int32 Y);

    //ReadOnly
    bool IsGameOver() const { return bGameOver; }
    bool IsWin() const { return bWin; }
    const FMinesweeperCell& GetCell(int32 X, int32 Y) const { return At(X, Y); }
    int32 GetWidth()  const { return Width; }
    int32 GetHeight() const { return Height; }
    const FMinesweeperConfig& GetConfig() const { return Config; }
    int32 GetTotalSafe() const { return Width * Height - Config.Bombs; }

private:
    //Grid Helpers
    bool IsValid(int32 X, int32 Y) const
    {
        return X >= 0 && X < Width && Y >= 0 && Y < Height;
    }

    FMinesweeperCell& At(int32 X, int32 Y)
    {
        check(IsValid(X, Y));
        return Cells[ToIndex(FCellCoord(X, Y), Width)];
    }
    const FMinesweeperCell & At(int32 X, int32 Y) const
    {
        check(IsValid(X, Y));
        return Cells[ToIndex(FCellCoord(X, Y), Width)];
    }
    

/*
 *Call func for each valid neighbors around
 *Func Callable with signature void(int32 NeighborX, int32 NeighborY)
 */
    template <typename Func>
    FORCEINLINE void ForEachNeighbor(int32 X, int32 Y, Func&& Fn) const
    {
        //Offset for the adjacency cell -> (-1,-1) (0,-1) (1,1) etc...
        static constexpr int NeighborOffsetX[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
        static constexpr int NeighborOffsetY[8] = { -1,-1,-1, 0, 0, 1, 1, 1 };
        
        for (int32 Index=0; Index<8; ++Index)
        {
            const int32 NeighborX  = X + NeighborOffsetX[Index];
            const int32 NeighborY = Y + NeighborOffsetY[Index];
            
            if (IsValid(NeighborX , NeighborY))
            {
                Fn(NeighborX , NeighborY);
            }
        }
    }

    
    //Core board logic
    void PlaceBombs();
    void ComputeAdjacency();
    
    //BFS flood from a zero-adjacency cell
    void FloodReveal(int32 X, int32 Y);
    //Helper for BFS
    void TryRevealSafeCell(int32 X, int32 Y, TQueue<FCellCoord>& Queue);

    //Relocate bombs (first click)
    void RelocateBombFrom(int32 X, int32 Y);

    
private:
    //Data
    FMinesweeperConfig   Config;
    int32                Width  = 0;
    int32                Height = 0;
    TArray<FMinesweeperCell> Cells;
    bool  bFirstMoveDone = false;

    //Game state
    bool  bGameOver = false;
    bool  bWin      = false;
    int32 RevealedSafeCells = 0;
};
