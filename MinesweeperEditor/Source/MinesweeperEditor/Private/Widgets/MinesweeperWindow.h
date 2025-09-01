#pragma once

#include "CoreMinimal.h"
#include "Board/MinesweeperBoard.h"
#include "Widgets/SCompoundWidget.h"
#include "Types/MinesweeperTypes.h"
#include "Widgets/Input/SSpinBox.h"

class SMinesweeperBoardView;

/*
 * Top-level editor window 
 *
 * Responsibilities:
 *  - Own the game state
 *  - Expose configuration controls
 *  - Host the drawing canvas
 */
class SMinesweeperWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMinesweeperWindow) {}
	SLATE_ATTRIBUTE(FMinesweeperConfig, InitialConfig)
SLATE_END_ARGS()

void Construct(const FArguments& InArgs);

private:
	//UI callbacks
	FReply OnNewGameClicked();
	void UpdateBombsMax();

	//Data
	FMinesweeperConfig Config;
	TSharedPtr<SSpinBox<int32>> BombsSpin;
	FMinesweeperBoard Board;
	TSharedPtr<SMinesweeperBoardView> BoardView;
};
