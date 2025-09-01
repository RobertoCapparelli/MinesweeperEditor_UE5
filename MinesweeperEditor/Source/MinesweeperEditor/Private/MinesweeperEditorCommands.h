#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"

class FMinesweeperEditorCommands : public TCommands<FMinesweeperEditorCommands>
{
public:

	FMinesweeperEditorCommands()
		: TCommands<FMinesweeperEditorCommands>(TEXT("MinesweeperEditor"), NSLOCTEXT("Contexts", "MinesweeperEditor", "MinesweeperEditor Plugin"), NAME_None, FAppStyle::GetAppStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
