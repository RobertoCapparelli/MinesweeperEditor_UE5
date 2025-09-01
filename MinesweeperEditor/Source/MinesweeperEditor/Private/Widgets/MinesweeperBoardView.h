#pragma once

#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "Fonts/SlateFontInfo.h"

class FMinesweeperBoard;



/*
 * Slate widget that draws a Minesweeper board
 *
 * Responsibilities:
 *  Render the grid using OnPaint()
 *  Map mouse position to cell coordinates
 */
class SMinesweeperBoardView : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SMinesweeperBoardView) {}
		// Non-owning pointer to the game board. Lifetime is managed by the window
		SLATE_ARGUMENT(FMinesweeperBoard*, Board)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	
	//SWidget overrides
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	                      const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	                      int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(400, 400); }

	//Mouse Events
	virtual FReply OnMouseButtonDown(const FGeometry& Geo, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseEnter(const FGeometry& Geo, const FPointerEvent& Evt) override;
	virtual void OnMouseLeave(const FPointerEvent& Evt) override;
	virtual FReply OnMouseMove(const FGeometry& Geo, const FPointerEvent& Evt) override;

private:
	//Layout helpers

	// Per-frame layout info used by painting and hit-testing
	struct FGridLayout
	{
		int32 Width = 0;
		int32 Height = 0;
		float Cell = 0.f;
		float GridWidth = 0.f;
		float GridHeight = 0.f;
		FVector2D Origin;
	};
	
	bool ComputeGridLayout(const FGeometry& Geo, FGridLayout& OutLayout) const;
	FIntPoint PosToCell(const FGeometry& Geo, const FVector2D& LocalPos) const;
	void EnsureSizeTextBombsForCell(const FGridLayout& Layout) const;

	// Per-number color mapping (1=blue, 2=green, 3..8=red)
	static FORCEINLINE FLinearColor NumColor(uint8 N)
	{
		if (N >= 3 && N <= 8)
		{
			return FLinearColor(0.9f, 0.2f, 0.2f, 1.f);
		}

		switch (N)
		{
		case 1: return FLinearColor(0.2f, 0.4f, 1.0f, 1.0f);
		case 2: return FLinearColor(0.2f, 0.7f, 0.3f, 1.0f);
		default: return FLinearColor::White;
		}
	}

	//DATA
	
	FMinesweeperBoard* Board = nullptr;
	
	//Currently hovered cell
	FIntPoint Hovered{-1, -1}; 
	
	static constexpr float PaddingCells = 1.0f;
	
	const FSlateBrush* Brush = nullptr;
	mutable FSlateFontInfo Font;
	
	TArray<FString> CachedText;
	mutable int32 CachedFontPx = -1;
	mutable FVector2D TextNumberBombsSize = FVector2D::ZeroVector;

};
