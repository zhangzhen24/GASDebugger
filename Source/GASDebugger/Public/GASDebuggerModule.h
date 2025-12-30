// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGASDebuggerSharedState;
class FGASDebuggerWindowInstance;
class FTabManager;

class FGASDebuggerModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Get the module instance */
	static FGASDebuggerModule& Get();

	/** Spawn a new debugger window (always creates a new instance) */
	void SpawnNewDebuggerWindow();

	/** Focus existing window or create new one if none exists */
	void FocusOrCreateDebuggerWindow();

	/** Get window instance by ID */
	TSharedPtr<FGASDebuggerWindowInstance> GetWindowInstance(int32 InstanceId) const;

	/** Get all active window instances */
	const TMap<int32, TSharedPtr<FGASDebuggerWindowInstance>>& GetWindowInstances() const { return WindowInstances; }

private:
	void RegisterMenus();
	void UnregisterMenus();
	void PluginButtonClicked();

	/** Create a debugger tab for a specific instance */
	TSharedRef<class SDockTab> OnSpawnDebuggerTab(const class FSpawnTabArgs& SpawnTabArgs, int32 InstanceId);

	/** Register child tabs for a window instance */
	void RegisterChildTabs(TSharedPtr<FGASDebuggerWindowInstance> Instance);

	/** Unregister child tabs for a window instance */
	void UnregisterChildTabs(TSharedPtr<FGASDebuggerWindowInstance> Instance);

	/** Remove a window instance */
	void RemoveWindowInstance(int32 InstanceId);

	/** Tab spawners for each category (with instance context) */
	TSharedRef<class SDockTab> SpawnTagsTab(const class FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak);
	TSharedRef<class SDockTab> SpawnAttributesTab(const class FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak);
	TSharedRef<class SDockTab> SpawnEffectsTab(const class FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak);
	TSharedRef<class SDockTab> SpawnAbilityTab(const class FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak);

	/** Command list for UI actions */
	TSharedPtr<class FUICommandList> PluginCommands;

	/** Multi-window management */
	TMap<int32, TSharedPtr<FGASDebuggerWindowInstance>> WindowInstances;
	int32 NextInstanceId = 0;
};
