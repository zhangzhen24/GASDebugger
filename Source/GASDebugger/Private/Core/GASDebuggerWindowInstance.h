// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FGASDebuggerSharedState;
class FTabManager;
class SDockTab;

/**
 * Encapsulates the state for a single GAS Debugger window instance.
 * Each window has its own SharedState, TabManager, and unique Tab IDs.
 */
class FGASDebuggerWindowInstance : public TSharedFromThis<FGASDebuggerWindowInstance>
{
public:
	FGASDebuggerWindowInstance(int32 InInstanceId);
	~FGASDebuggerWindowInstance();

	// Getters
	int32 GetInstanceId() const { return InstanceId; }

	// Tab ID generation - each window has unique Tab IDs
	FName GetMainTabId() const;
	FName GetAbilityTabId() const;
	FName GetTagsTabId() const;
	FName GetAttributesTabId() const;
	FName GetEffectsTabId() const;

	// State accessors
	TSharedPtr<FGASDebuggerSharedState> GetSharedState() const { return SharedState; }
	TSharedPtr<FTabManager> GetTabManager() const { return TabManager; }
	TSharedPtr<FTabManager::FLayout> GetTabLayout() const { return TabLayout; }

	// Get window title
	FText GetWindowTitle() const;

	// Layout persistence key
	FString GetLayoutConfigKey() const;

	// Lifecycle
	void Initialize(TSharedRef<SDockTab> OwnerTab);
	void Shutdown();

	// Set TabLayout after creation
	void SetTabLayout(TSharedPtr<FTabManager::FLayout> InLayout) { TabLayout = InLayout; }

private:
	int32 InstanceId;
	TSharedPtr<FGASDebuggerSharedState> SharedState;
	TSharedPtr<FTabManager> TabManager;
	TSharedPtr<FTabManager::FLayout> TabLayout;
	TWeakPtr<SDockTab> MainTabWeak;
};
