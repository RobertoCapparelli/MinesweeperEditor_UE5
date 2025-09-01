#include "MinesweeperEditorCommands.h"

#define LOCTEXT_NAMESPACE "FMinesweeperEditorModule"

void FMinesweeperEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "Minesweeper", "Open the Minesweeper tool", EUserInterfaceActionType::Button, FInputChord(EModifierKey::Control | EModifierKey::Shift, EKeys::M));
}

#undef LOCTEXT_NAMESPACE
