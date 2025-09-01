#include "Widgets/MinesweeperWindow.h"

#include "Utility/MinesweeperEditorLog.h"
#include "Utility/MinesweeperNotification.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/MinesweeperBoardView.h"

#define LOCTEXT_NAMESPACE "SMinesweeperWindow"

void SMinesweeperWindow::Construct(const FArguments& InArgs)
{
	Config = InArgs._InitialConfig.Get(FMinesweeperConfig{});
	Board.StartNewGame(Config);
	FMinesweeperNotification::Show(LOCTEXT("MSGStarted", "New game started!"));

	ChildSlot
	[
		SNew(SVerticalBox)
		
		//Row
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FMargin(4, 0))

			//Width
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Width", "Width"))
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SSpinBox<int32>)
				.MinValue(Limits::MinWidth)
				.MaxValue(Limits::MaxWidth)
				.MinSliderValue(Limits::MinWidth)
				.MaxSliderValue(Limits::MaxWidth)
				.ToolTipText(LOCTEXT("Width", "Grid width (columns)"))
				.Value(Config.Width)
				.OnValueChanged_Lambda([this](int32 Value)
				{
					Config.Width = Value;
					UpdateBombsMax();
				})
			]

			//Height
			+ SUniformGridPanel::Slot(0, 1)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Height", "Height"))
			]
			+ SUniformGridPanel::Slot(1, 1)
			[
				SNew(SSpinBox<int32>)
				.MinValue(Limits::MinHeight)
				.MaxValue(Limits::MaxHeight)
				.MinSliderValue(Limits::MinHeight)
				.MaxSliderValue(Limits::MaxHeight)
				.Value(Config.Height)
				.ToolTipText(LOCTEXT("Height", "Grid height (rows)"))
				.OnValueChanged_Lambda([this](int32 Value)
				{
					Config.Height = Value;
					UpdateBombsMax();
				})
			]

			//Bombs
			+ SUniformGridPanel::Slot(0, 2)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Bombs", "Bombs"))
			]
			+ SUniformGridPanel::Slot(1, 2)
			[
				SAssignNew(BombsSpin, SSpinBox<int32>)
				.MinValue(Limits::MinBombs)
				.MaxValue(Limits::MaxBombsFor(Config.Width, Config.Height))
				.MinSliderValue(Limits::MinBombs)
				.MaxSliderValue(Limits::MaxBombsFor(Config.Width, Config.Height))
				.Delta(1)
				.MinDesiredWidth(90)
				.Value(Config.Bombs)
				.ToolTipText(LOCTEXT("Bombs", "Number of bombs (at least one safe cell is enforced)"))
				.OnValueChanged_Lambda([this](int32 Value)
				{
					Config.Bombs = Value;
				})
			]

		]

		//New Game button 
		+ SVerticalBox::Slot().AutoHeight().Padding(8)
		[
			SNew(SButton)
			.Text(LOCTEXT("NewGame", "New Game"))
			.ToolTipText(LOCTEXT("NewGame", "Start a new game with the current settings"))
			.OnClicked(this, &SMinesweeperWindow::OnNewGameClicked)
		]

		//Board
		+ SVerticalBox::Slot()
		.Padding(8)
		.FillHeight(1.f)
		[
			SAssignNew(BoardView, SMinesweeperBoardView)
			.Board(&Board)
		]
	];
	// Sync bombs max with initial width/height
	UpdateBombsMax();
}

/*
 * Start a new game and repaint the board
 */
FReply SMinesweeperWindow::OnNewGameClicked()
{
	//Check or invalids inputs are check in the board itself (Limits::)
	
	Board.StartNewGame(Config);
	if (BoardView.IsValid())
	{
		BoardView->Invalidate(EInvalidateWidgetReason::Paint);
	}

	//Notify message
	FMinesweeperNotification::Show(LOCTEXT("MSGStarted", "New game started"));
	
	return FReply::Handled();
}

/*
 * Keep the Bombs spinbox in sync with Width and Height
 */
void SMinesweeperWindow::UpdateBombsMax()
{
	if (!ensureMsgf(BombsSpin.IsValid(), TEXT("BombsSpin is invalid in UpdateBombsMax")))
	{
		return;
	}
	
	const int32 MaxBombs = Limits::MaxBombsFor(Config.Width, Config.Height);
	
	BombsSpin->SetMaxValue(MaxBombs);
	BombsSpin->SetMaxSliderValue(MaxBombs);
	BombsSpin->SetMinValue(Limits::MinBombs);
	BombsSpin->SetMinSliderValue(Limits::MinBombs);
	
	Config.Bombs = FMath::Clamp(Config.Bombs, Limits::MinBombs, MaxBombs);
	BombsSpin->SetValue(Config.Bombs);
}

#undef LOCTEXT_NAMESPACE
