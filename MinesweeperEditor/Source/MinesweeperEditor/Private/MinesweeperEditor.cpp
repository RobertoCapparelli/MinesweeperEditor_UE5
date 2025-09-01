#include "MinesweeperEditor.h"
#include "MinesweeperEditorCommands.h"
#include "Widgets/MinesweeperWindow.h"
#include "Utility/MinesweeperEditorLog.h"
#include "LevelEditor.h"
#include "ToolMenus.h"

static const FName MinesweeperEditorTabName("MinesweeperEditor");
DEFINE_LOG_CATEGORY(LogMinesweeper);

#define LOCTEXT_NAMESPACE "FMinesweeperEditorModule"

void FMinesweeperEditorModule::StartupModule()
{
	FMinesweeperEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	//Map Action for button/menu
	PluginCommands->MapAction(
		FMinesweeperEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FMinesweeperEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	//Map Action for shortcut
	FLevelEditorModule& LevelEditorModule =
		FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	const TSharedPtr<FUICommandList> LevelEditorCommands = LevelEditorModule.GetGlobalLevelEditorActions();

	LevelEditorCommands->MapAction(
		FMinesweeperEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FMinesweeperEditorModule::PluginButtonClicked),
		FCanExecuteAction()
	);
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		MinesweeperEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FMinesweeperEditorModule::OnSpawnPluginTab)
	)
	.SetDisplayName(LOCTEXT("MinesweeperTabTitle","Minesweeper"))
	.SetMenuType(ETabSpawnerMenuType::Hidden);
	
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FMinesweeperEditorModule::RegisterMenus));
}

void FMinesweeperEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FMinesweeperEditorCommands::Unregister();
}

void FMinesweeperEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(MinesweeperEditorTabName);
}
TSharedRef<SDockTab> FMinesweeperEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SMinesweeperWindow)
		];
}


void FMinesweeperEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FMinesweeperEditorCommands::Get().PluginAction, PluginCommands);
	}

	if (UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"))
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");

		FToolMenuEntry& Entry = Section.AddEntry(
			FToolMenuEntry::InitToolBarButton(
				FMinesweeperEditorCommands::Get().PluginAction,
				TAttribute<FText>(),
				TAttribute<FText>(),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Plus")
			)
		);
		Entry.SetCommandList(PluginCommands);
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMinesweeperEditorModule, MinesweeperEditor)