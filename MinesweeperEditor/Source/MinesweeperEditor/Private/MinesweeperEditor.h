#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMinesweeperEditorModule : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void PluginButtonClicked();
	
private:

	void RegisterMenus();
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs);

	TSharedPtr<FUICommandList> PluginCommands;
};
