// Copyright Qiu, Inc. All Rights Reserved.

#include "GASDebuggerModule.h"
#include "GASDebuggerStyle.h"
#include "GASDebuggerCommands.h"
#include "Core/GASDebuggerSharedState.h"
#include "Core/GASDebuggerWindowInstance.h"
#include "Widgets/SGASDebuggerMainWindow.h"
#include "Widgets/Tabs/SGASDebuggerTagsTab.h"
#include "Widgets/Tabs/SGASDebuggerAttributesTab.h"
#include "Widgets/Tabs/SGASDebuggerEffectsTab.h"
#include "Widgets/Tabs/SGASDebuggerAbilityTab.h"

#if WITH_EDITOR
#include "LevelEditor.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "ToolMenus.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/Docking/LayoutService.h"
#include "Widgets/Docking/SDockTab.h"
#include "Styling/AppStyle.h"
#endif

#define LOCTEXT_NAMESPACE "FGASDebuggerModule"

void FGASDebuggerModule::StartupModule()
{
#if WITH_EDITOR
	// Initialize style and commands
	FGASDebuggerStyle::Initialize();
	FGASDebuggerStyle::ReloadTextures();
	FGASDebuggerCommands::Register();

	// Create command list
	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FGASDebuggerCommands::Get().OpenGASDebugger_Button,
		FExecuteAction::CreateRaw(this, &FGASDebuggerModule::PluginButtonClicked),
		FCanExecuteAction());

	// Register menus
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FGASDebuggerModule::RegisterMenus));
#endif
}

void FGASDebuggerModule::ShutdownModule()
{
#if WITH_EDITOR
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FGASDebuggerStyle::Shutdown();
	FGASDebuggerCommands::Unregister();

	// Close all window instances
	TArray<int32> InstanceIds;
	WindowInstances.GetKeys(InstanceIds);
	for (int32 InstanceId : InstanceIds)
	{
		if (TSharedPtr<FGASDebuggerWindowInstance> Instance = WindowInstances.FindRef(InstanceId))
		{
			UnregisterChildTabs(Instance);
			FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Instance->GetMainTabId());
			Instance->Shutdown();
		}
	}
	WindowInstances.Empty();
#endif
}

FGASDebuggerModule& FGASDebuggerModule::Get()
{
	return FModuleManager::LoadModuleChecked<FGASDebuggerModule>("GASDebugger");
}

void FGASDebuggerModule::SpawnNewDebuggerWindow()
{
#if WITH_EDITOR
	int32 InstanceId = NextInstanceId++;

	// Create window instance
	TSharedPtr<FGASDebuggerWindowInstance> Instance = MakeShared<FGASDebuggerWindowInstance>(InstanceId);
	WindowInstances.Add(InstanceId, Instance);

	FName TabName = Instance->GetMainTabId();

	// Register NomadTabSpawner for this instance
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TabName,
		FOnSpawnTab::CreateRaw(this, &FGASDebuggerModule::OnSpawnDebuggerTab, InstanceId))
		.SetDisplayName(Instance->GetWindowTitle())
		.SetTooltipText(LOCTEXT("GASDebuggerTabTooltip", "Gameplay Ability System Debugger"))
		.SetGroup(WorkspaceMenu::GetMenuStructure().GetDeveloperToolsDebugCategory())
		.SetIcon(FSlateIcon(FGASDebuggerStyle::GetStyleSetName(), "GASDebugger.OpenGASDebugger"));

	// Open the tab
	FGlobalTabmanager::Get()->TryInvokeTab(TabName);
#endif
}

void FGASDebuggerModule::FocusOrCreateDebuggerWindow()
{
#if WITH_EDITOR
	// If any window exists, focus the first one
	if (WindowInstances.Num() > 0)
	{
		for (auto& Pair : WindowInstances)
		{
			TSharedPtr<SDockTab> ExistingTab = FGlobalTabmanager::Get()->FindExistingLiveTab(Pair.Value->GetMainTabId());
			if (ExistingTab.IsValid())
			{
				ExistingTab->DrawAttention();
				FGlobalTabmanager::Get()->TryInvokeTab(Pair.Value->GetMainTabId());
				return;
			}
		}
	}

	// No window exists, create a new one
	SpawnNewDebuggerWindow();
#endif
}

TSharedPtr<FGASDebuggerWindowInstance> FGASDebuggerModule::GetWindowInstance(int32 InstanceId) const
{
	return WindowInstances.FindRef(InstanceId);
}

void FGASDebuggerModule::PluginButtonClicked()
{
#if WITH_EDITOR
	// Focus existing window or create new one
	FocusOrCreateDebuggerWindow();
#endif
}

#if WITH_EDITOR
void FGASDebuggerModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to Window menu
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FGASDebuggerCommands::Get().OpenGASDebugger_Button, PluginCommands);
	}

	// Add toolbar button
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FGASDebuggerCommands::Get().OpenGASDebugger_Button));
		Entry.SetCommandList(PluginCommands);
		Entry.Name = "OpenGASDebugger";
		Entry.Label = LOCTEXT("ToolbarButtonLabel", "GAS Debugger");
		Entry.ToolTip = LOCTEXT("ToolbarButtonTooltip", "Open the GAS Debugger window");
	}
}

void FGASDebuggerModule::UnregisterMenus()
{
	// Cleanup handled by UToolMenus::UnregisterOwner in ShutdownModule
}

TSharedRef<SDockTab> FGASDebuggerModule::OnSpawnDebuggerTab(const FSpawnTabArgs& SpawnTabArgs, int32 InstanceId)
{
	// Get the window instance
	TSharedPtr<FGASDebuggerWindowInstance> Instance = WindowInstances.FindRef(InstanceId);
	if (!Instance.IsValid())
	{
		// Instance should have been created in SpawnNewDebuggerWindow, but handle edge case
		Instance = MakeShared<FGASDebuggerWindowInstance>(InstanceId);
		WindowInstances.Add(InstanceId, Instance);
	}

	// Create the main NomadTab
	TSharedRef<SDockTab> NomadTab = SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		.Label(Instance->GetWindowTitle());

	// Initialize the window instance with the tab
	Instance->Initialize(NomadTab);

	TSharedPtr<FTabManager> TabManager = Instance->GetTabManager();

	// Register child tabs for this instance
	RegisterChildTabs(Instance);

	// Define default layout:
	// +------------+------+------------+
	// |            |      |            |
	// |            |      |   Effect   |
	// |            |      |            |
	// |  Ability   | Tags +------------+
	// |            |      |            |
	// |            |      |   Attr     |
	// |            |      |            |
	// +------------+------+------------+
	FName LayoutName = *Instance->GetLayoutConfigKey();
	TSharedPtr<FTabManager::FLayout> TabLayout = FTabManager::NewLayout(LayoutName)
		->AddArea
		(
			FTabManager::NewPrimaryArea()
			->SetOrientation(Orient_Horizontal)
			->Split
			(
				// Left: Ability (35%)
				FTabManager::NewStack()
				->SetSizeCoefficient(0.35f)
				->AddTab(Instance->GetAbilityTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				// Middle: Tags (15%)
				FTabManager::NewStack()
				->SetSizeCoefficient(0.15f)
				->AddTab(Instance->GetTagsTabId(), ETabState::OpenedTab)
			)
			->Split
			(
				// Right: Effects on top, Attributes on bottom (50%)
				FTabManager::NewSplitter()
				->SetOrientation(Orient_Vertical)
				->SetSizeCoefficient(0.5f)
				->Split
				(
					// Top: Effects (50%)
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab(Instance->GetEffectsTabId(), ETabState::OpenedTab)
				)
				->Split
				(
					// Bottom: Attributes (50%)
					FTabManager::NewStack()
					->SetSizeCoefficient(0.5f)
					->AddTab(Instance->GetAttributesTabId(), ETabState::OpenedTab)
				)
			)
		);

	Instance->SetTabLayout(TabLayout);

	// Restore the layout
	TSharedRef<SWidget> TabContents = TabManager->RestoreFrom(
		TabLayout.ToSharedRef(),
		TSharedPtr<SWindow>()
	).ToSharedRef();

	// Capture for lambda
	TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak = Instance;
	int32 CapturedInstanceId = InstanceId;

	// Set up tab close callback
	NomadTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda(
		[this, InstanceWeak, CapturedInstanceId](TSharedRef<SDockTab>)
		{
			if (TSharedPtr<FGASDebuggerWindowInstance> Inst = InstanceWeak.Pin())
			{
				// Unregister child tabs and cleanup
				UnregisterChildTabs(Inst);
				FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Inst->GetMainTabId());
				Inst->Shutdown();
			}

			WindowInstances.Remove(CapturedInstanceId);
		}
	));

	// Combine TopBar + TabContents
	NomadTab->SetContent(
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SGASDebuggerMainWindow)
			.SharedState(Instance->GetSharedState())
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			TabContents
		]
	);

	return NomadTab;
}

void FGASDebuggerModule::RegisterChildTabs(TSharedPtr<FGASDebuggerWindowInstance> Instance)
{
	if (!Instance.IsValid() || !Instance->GetTabManager().IsValid())
	{
		return;
	}

	TSharedPtr<FTabManager> TabManager = Instance->GetTabManager();
	TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak = Instance;

	// Tags Tab
	TabManager->RegisterTabSpawner(
		Instance->GetTagsTabId(),
		FOnSpawnTab::CreateRaw(this, &FGASDebuggerModule::SpawnTagsTab, InstanceWeak))
		.SetDisplayName(SGASDebuggerTagsTab::GetTabLabel())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	// Attributes Tab
	TabManager->RegisterTabSpawner(
		Instance->GetAttributesTabId(),
		FOnSpawnTab::CreateRaw(this, &FGASDebuggerModule::SpawnAttributesTab, InstanceWeak))
		.SetDisplayName(SGASDebuggerAttributesTab::GetTabLabel())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	// Effects Tab
	TabManager->RegisterTabSpawner(
		Instance->GetEffectsTabId(),
		FOnSpawnTab::CreateRaw(this, &FGASDebuggerModule::SpawnEffectsTab, InstanceWeak))
		.SetDisplayName(SGASDebuggerEffectsTab::GetTabLabel())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));

	// Ability Tab
	TabManager->RegisterTabSpawner(
		Instance->GetAbilityTabId(),
		FOnSpawnTab::CreateRaw(this, &FGASDebuggerModule::SpawnAbilityTab, InstanceWeak))
		.SetDisplayName(SGASDebuggerAbilityTab::GetTabLabel())
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FGASDebuggerModule::UnregisterChildTabs(TSharedPtr<FGASDebuggerWindowInstance> Instance)
{
	if (!Instance.IsValid() || !Instance->GetTabManager().IsValid())
	{
		return;
	}

	TSharedPtr<FTabManager> TabManager = Instance->GetTabManager();

	TabManager->UnregisterTabSpawner(Instance->GetTagsTabId());
	TabManager->UnregisterTabSpawner(Instance->GetAttributesTabId());
	TabManager->UnregisterTabSpawner(Instance->GetEffectsTabId());
	TabManager->UnregisterTabSpawner(Instance->GetAbilityTabId());
}

void FGASDebuggerModule::RemoveWindowInstance(int32 InstanceId)
{
	if (TSharedPtr<FGASDebuggerWindowInstance> Instance = WindowInstances.FindRef(InstanceId))
	{
		UnregisterChildTabs(Instance);
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(Instance->GetMainTabId());
		Instance->Shutdown();
		WindowInstances.Remove(InstanceId);
	}
}

TSharedRef<SDockTab> FGASDebuggerModule::SpawnTagsTab(const FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak)
{
	TSharedPtr<FGASDebuggerWindowInstance> Instance = InstanceWeak.Pin();
	TSharedPtr<FGASDebuggerSharedState> SharedState = Instance.IsValid() ? Instance->GetSharedState() : nullptr;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(SGASDebuggerTagsTab::GetTabLabel())
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f)))
			[
				SNew(SGASDebuggerTagsTab)
				.SharedState(SharedState)
			]
		];
}

TSharedRef<SDockTab> FGASDebuggerModule::SpawnAttributesTab(const FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak)
{
	TSharedPtr<FGASDebuggerWindowInstance> Instance = InstanceWeak.Pin();
	TSharedPtr<FGASDebuggerSharedState> SharedState = Instance.IsValid() ? Instance->GetSharedState() : nullptr;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(SGASDebuggerAttributesTab::GetTabLabel())
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f)))
			[
				SNew(SGASDebuggerAttributesTab)
				.SharedState(SharedState)
			]
		];
}

TSharedRef<SDockTab> FGASDebuggerModule::SpawnEffectsTab(const FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak)
{
	TSharedPtr<FGASDebuggerWindowInstance> Instance = InstanceWeak.Pin();
	TSharedPtr<FGASDebuggerSharedState> SharedState = Instance.IsValid() ? Instance->GetSharedState() : nullptr;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(SGASDebuggerEffectsTab::GetTabLabel())
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f)))
			[
				SNew(SGASDebuggerEffectsTab)
				.SharedState(SharedState)
			]
		];
}

TSharedRef<SDockTab> FGASDebuggerModule::SpawnAbilityTab(const FSpawnTabArgs& Args, TWeakPtr<FGASDebuggerWindowInstance> InstanceWeak)
{
	TSharedPtr<FGASDebuggerWindowInstance> Instance = InstanceWeak.Pin();
	TSharedPtr<FGASDebuggerSharedState> SharedState = Instance.IsValid() ? Instance->GetSharedState() : nullptr;

	return SNew(SDockTab)
		.TabRole(ETabRole::PanelTab)
		.Label(SGASDebuggerAbilityTab::GetTabLabel())
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("Docking.Tab.ContentAreaBrush"))
			.BorderBackgroundColor(FSlateColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.f)))
			[
				SNew(SGASDebuggerAbilityTab)
				.SharedState(SharedState)
			]
		];
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGASDebuggerModule, GASDebugger)
