// Copyright Qiu, Inc. All Rights Reserved.

#include "Core/GASDebuggerWindowInstance.h"
#include "Core/GASDebuggerSharedState.h"
#include "Framework/Docking/TabManager.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "GASDebuggerWindowInstance"

FGASDebuggerWindowInstance::FGASDebuggerWindowInstance(int32 InInstanceId)
	: InstanceId(InInstanceId)
{
	SharedState = MakeShared<FGASDebuggerSharedState>();
}

FGASDebuggerWindowInstance::~FGASDebuggerWindowInstance()
{
	Shutdown();
}

FName FGASDebuggerWindowInstance::GetMainTabId() const
{
	return *FString::Printf(TEXT("GASDebugger_%d"), InstanceId);
}

FName FGASDebuggerWindowInstance::GetAbilityTabId() const
{
	return *FString::Printf(TEXT("GASDebugger_%d_Ability"), InstanceId);
}

FName FGASDebuggerWindowInstance::GetTagsTabId() const
{
	return *FString::Printf(TEXT("GASDebugger_%d_Tags"), InstanceId);
}

FName FGASDebuggerWindowInstance::GetAttributesTabId() const
{
	return *FString::Printf(TEXT("GASDebugger_%d_Attributes"), InstanceId);
}

FName FGASDebuggerWindowInstance::GetEffectsTabId() const
{
	return *FString::Printf(TEXT("GASDebugger_%d_Effects"), InstanceId);
}

FText FGASDebuggerWindowInstance::GetWindowTitle() const
{
	return FText::Format(LOCTEXT("WindowTitle", "GAS Debugger #{0}"), FText::AsNumber(InstanceId + 1));
}

FString FGASDebuggerWindowInstance::GetLayoutConfigKey() const
{
	return FString::Printf(TEXT("GASDebugger_Layout_%d"), InstanceId);
}

void FGASDebuggerWindowInstance::Initialize(TSharedRef<SDockTab> OwnerTab)
{
	MainTabWeak = OwnerTab;

	// Create TabManager for child tabs
	TabManager = FGlobalTabmanager::Get()->NewTabManager(OwnerTab);
}

void FGASDebuggerWindowInstance::Shutdown()
{
	if (TabManager.IsValid())
	{
		TabManager->CloseAllAreas();
		TabManager.Reset();
	}

	TabLayout.Reset();
	SharedState.Reset();
	MainTabWeak.Reset();
}

#undef LOCTEXT_NAMESPACE
