#include "Widgets/MinesweeperBoardView.h"
#include "Board/MinesweeperBoard.h"
#include "Fonts/FontMeasure.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Utility/MinesweeperNotification.h"
#include "Fonts/SlateFontInfo.h"

#define LOCTEXT_NAMESPACE "SMinesweeperBoardView"

/*
 * - Caches a default brush and font from the app style
 * - Precomputes string literals "0..8" (we'll reuse index-based access later)
 */
void SMinesweeperBoardView::Construct(const FArguments& InArgs)
{
	Board = InArgs._Board;

	Brush = FAppStyle::Get().GetBrush("WhiteBrush");
	Font = FAppStyle::Get().GetFontStyle("NormalText");

	//Cache number text (0..8) for text number bombs in cells
	CachedText.SetNum(9);
	for (int Index = 0; Index <= 8; ++Index)
	{
		CachedText[Index] = FString::FromInt(Index);
	}
}


int32 SMinesweeperBoardView::OnPaint(const FPaintArgs& Args,
                                     const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
                                     FSlateWindowElementList& OutDrawElements,
                                     int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	//Compute GridLayout: Width, Height, Cell(size), GridWidth, GridHeight, Origin
	FGridLayout Layout;
	if (!ComputeGridLayout(AllottedGeometry, Layout))
	{
		return LayerId;
	}
	//Helper to paint geometry 
	auto PaintGeometry = [&AllottedGeometry](const FVector2D& Position, const FVector2D& Size)
	{
		return AllottedGeometry.ToPaintGeometry(
			FVector2f(Size),
			FSlateLayoutTransform(FVector2f(Position))
		);
	};

	//Font sizing & cached text size for digits
	EnsureSizeTextBombsForCell(Layout);


	// Draw each cell, background and optional number
	for (int YIndex = 0; YIndex < Layout.Height; ++YIndex)
	{
		for (int XIndex = 0; XIndex < Layout.Width; ++XIndex)
		{
			//Get the current cell using cords x,y
			const FMinesweeperCell& CurrentCell = Board->GetCell(XIndex, YIndex);

			//Compute the origin and the size of the current cell  
			const FVector2D PositionCurrentCell = Layout.Origin + FVector2D(XIndex * Layout.Cell, YIndex * Layout.Cell);
			const FVector2D SizeCells(Layout.Cell, Layout.Cell);

			//Using the padding for space between cell
			const FVector2D PositionCurrentCellInner = PositionCurrentCell + FVector2D(PaddingCells, PaddingCells);
			const FVector2D SizeCellsInner = SizeCells - FVector2D(PaddingCells * 2.f, PaddingCells * 2.f);

			//Using color based on the state
			const FLinearColor Fill =
				(CurrentCell.State == ETileState::Hidden)
					? FLinearColor(0.25f, 0.25f, 0.25f, 1)
					: (CurrentCell.State == ETileState::Exploded)
					? FLinearColor(0.85f, 0.1f, 0.1f, 1)
					: FLinearColor(0.35f, 0.35f, 0.35f, 1);

			//Drawing the cell 
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
			                           PaintGeometry(PositionCurrentCellInner, SizeCellsInner),
			                           Brush, ESlateDrawEffect::None, Fill);

			//If the cell is revealed and have adjacent bombs show the number of the bombs adjacent
			if (CurrentCell.State == ETileState::Revealed && CurrentCell.AdjacentBombs > 0)
			{
				const uint8 NumberBombsAdj = CurrentCell.AdjacentBombs;
				const FVector2D Center = PositionCurrentCell + (SizeCells - TextNumberBombsSize) * 0.5f;

				FSlateDrawElement::MakeText(
					OutDrawElements, LayerId + 1, PaintGeometry(Center, TextNumberBombsSize),
					CachedText[NumberBombsAdj], Font, ESlateDrawEffect::None, NumColor(NumberBombsAdj));
			}
		}
	}

	//END GAME
	if (Board->IsGameOver() || Board->IsWin())
	{
		const FString EndGameText = Board->IsGameOver() ? TEXT("Game Over") : TEXT("You Win!");

		// Larger font for the overlay message
		FSlateFontInfo OverlayFontBig = Font;
		OverlayFontBig.Size = FMath::Clamp(Font.Size * 2, 12, 64);

		// Dark veil over the grid area
		FSlateDrawElement::MakeBox(
			OutDrawElements, LayerId + 4, PaintGeometry(Layout.Origin, FVector2D(Layout.GridWidth, Layout.GridHeight)),
			Brush, ESlateDrawEffect::None, FLinearColor(0, 0, 0, 0.45f)
		);

		// Center the text in the grid rect
		const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
		const FVector2D TextSize = Measure->Measure(EndGameText, OverlayFontBig);
		const FVector2D TextCenter = Layout.Origin + FVector2D(Layout.GridWidth, Layout.GridHeight) * 0.5f - TextSize *
			0.5f;

		FSlateDrawElement::MakeText(
			OutDrawElements, LayerId + 5, PaintGeometry(TextCenter, TextSize),
			EndGameText, OverlayFontBig, ESlateDrawEffect::None, FLinearColor::White
		);
		return LayerId + 6;
	}

	// Hover overlay
	if (Hovered.X >= 0 && Hovered.Y >= 0 && Hovered.X < Layout.Width && Hovered.Y < Layout.Height)
	{
		const FVector2D HoverPosition = Layout.Origin + FVector2D(Hovered.X * Layout.Cell, Hovered.Y * Layout.Cell);
		const FVector2D HoverSize(Layout.Cell, Layout.Cell);

		// light fill
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 2,
		                           PaintGeometry(HoverPosition, HoverSize),
		                           Brush, ESlateDrawEffect::None, FLinearColor(1, 1, 1, 0.08f));
		// Soft edge
		FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 3,
		                           PaintGeometry(HoverPosition, HoverSize),
		                           Brush, ESlateDrawEffect::None, FLinearColor(0.9f, 0.9f, 0.9f, 0.6f));
	}

	return LayerId + 4;
}

/*
 * Ensure the cached font size (and measured text size) matches the current cell size
 * Uses "8" as the widest digit to size text boxes consistently
 */
void SMinesweeperBoardView::EnsureSizeTextBombsForCell(const FGridLayout& Layout) const
{
	//50% of the cell size
	constexpr float FontSizeMul = 0.5f;
	const int32 FontPX = FMath::Clamp(FMath::RoundToInt(Layout.Cell * FontSizeMul), 8, 32);

	//On tab size change or new gird, compute the font size
	if (FontPX != CachedFontPx)
	{
		CachedFontPx = FontPX;
		Font.Size = CachedFontPx;

		if (FSlateApplication::IsInitialized())
		{
			const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
			TextNumberBombsSize = Measure->Measure(TEXT("8"), Font);
		}
	}
}

/*
 * Compute grid layout (Width/Height, per-cell size, total grid rect, origin)
 * Returns false if Board is null/invalid or computed cell size < 1 SU 
 */
bool SMinesweeperBoardView::ComputeGridLayout(const FGeometry& Geo, FGridLayout& OutLayout) const
{
	if (Board == nullptr)
	{
		return false;
	}

	OutLayout.Width = Board->GetWidth();
	OutLayout.Height = Board->GetHeight();


	if (OutLayout.Width <= 0 || OutLayout.Height <= 0)
	{
		return false;
	}

	//Compute cell size in Slate Units. If it's < 1 SU, skip painting
	const FVector2D Size = Geo.GetLocalSize();
	OutLayout.Cell = FMath::FloorToFloat(FMath::Min(Size.X / OutLayout.Width, Size.Y / OutLayout.Height));
	if (OutLayout.Cell <= 0.f)
	{
		return false;
	}

	//Compute the origin of the grid
	OutLayout.GridWidth = OutLayout.Cell * OutLayout.Width;
	OutLayout.GridHeight = OutLayout.Cell * OutLayout.Height;
	OutLayout.Origin = FVector2D((Size.X - OutLayout.GridWidth) * 0.5f, (Size.Y - OutLayout.GridHeight) * 0.5f);
	return true;
}

/*
 * Convert a local widget position to grid cell coordinates (X,Y)
 * Returns (-1,-1) when outside the grid
 */
FIntPoint SMinesweeperBoardView::PosToCell(const FGeometry& Geo, const FVector2D& LocalPos) const
{
	//Get current Layout
	FGridLayout Layout;
	if (!ComputeGridLayout(Geo, Layout))
	{
		return FIntPoint(-1, -1);
	}
	//Get mouse position inner the grid
	const FVector2D Position = LocalPos - Layout.Origin;

	const int32 X = FMath::FloorToInt(Position.X / Layout.Cell);
	const int32 Y = FMath::FloorToInt(Position.Y / Layout.Cell);

	//If index are inside the limits 
	return (X >= 0 && X < Layout.Width && Y >= 0 && Y < Layout.Height)
		       ? FIntPoint(X, Y)
		       : FIntPoint(-1, -1);
}


//MOUSE EVENT

// translate mouse position to cell, reveal it, and invalidate for repaint 
FReply SMinesweeperBoardView::OnMouseButtonDown(const FGeometry& Geo, const FPointerEvent& MouseEvent)
{
	if (!Board)
	{
		return FReply::Unhandled();
	}

	if (Board->IsGameOver() || Board->IsWin())
	{
		return FReply::Unhandled();
	}

	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	const FVector2D Local = Geo.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const FIntPoint Cell = PosToCell(Geo, Local);

	if (Cell.X >= 0 && Cell.Y >= 0 && Cell.X < Board->GetWidth() && Cell.Y < Board->GetHeight())
	{
		const auto Outcome = Board->Reveal(Cell.X, Cell.Y);
		Invalidate(EInvalidateWidgetReason::Paint);

		//Notify MSG
		if (Outcome == FMinesweeperBoard::ERevealOutcome::Exploded)
		{
			FMinesweeperNotification::Show(LOCTEXT("MSGGameOver", "Game Over"), SNotificationItem::CS_Fail);
		}
		else if (Board->IsWin())
		{
			FMinesweeperNotification::Show(LOCTEXT("MSGWin", "You Win"), SNotificationItem::CS_Success);
		}
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

void SMinesweeperBoardView::OnMouseEnter(const FGeometry& Geo, const FPointerEvent& Evt)
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

void SMinesweeperBoardView::OnMouseLeave(const FPointerEvent& Evt)
{
	if (Hovered.X != -1 || Hovered.Y != -1)
	{
		Hovered = FIntPoint(-1, -1);
		Invalidate(EInvalidateWidgetReason::Paint);
	}
}

FReply SMinesweeperBoardView::OnMouseMove(const FGeometry& Geo, const FPointerEvent& Evt)
{
	if (!Board)
	{
		return FReply::Unhandled();
	}
	
	const FVector2D Local = Geo.AbsoluteToLocal(Evt.GetScreenSpacePosition());
	const FIntPoint Cell = PosToCell(Geo, Local);

	// Invalid if cell is changed
	if (Cell != Hovered)
	{
		Hovered = Cell;
		Invalidate(EInvalidateWidgetReason::Paint);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
