// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/SGASDebuggerMainWindow.h"
#include "Core/GASDebuggerSharedState.h"
#include "GASDebuggerModule.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"

#define LOCTEXT_NAMESPACE "SGASDebuggerMainWindow"

//////////////////////////////////////////////////////////////////////////
// FGASDebuggerInputProcessor

FGASDebuggerInputProcessor::FGASDebuggerInputProcessor(TSharedPtr<FGASDebuggerSharedState> InSharedState)
	: SharedStateWeak(InSharedState)
{
}

FGASDebuggerInputProcessor::~FGASDebuggerInputProcessor()
{
}

bool FGASDebuggerInputProcessor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::End)
	{
		TSharedPtr<FGASDebuggerSharedState> SharedState = SharedStateWeak.Pin();
		if (SharedState.IsValid())
		{
			SharedState->SetPickingMode(false);
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// SGASDebuggerMainWindow

void SGASDebuggerMainWindow::Construct(const FArguments& InArgs)
{
	SharedState = InArgs._SharedState;

	ChildSlot
	[
		BuildTopBar()
	];

	// Register input processor for END key
	InputProcessor = MakeShareable(new FGASDebuggerInputProcessor(SharedState));
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);

	// Initialize
	if (SharedState.IsValid())
	{
		SharedState->RefreshASCList();
	}
}

SGASDebuggerMainWindow::~SGASDebuggerMainWindow()
{
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		InputProcessor.Reset();
	}
}

void SGASDebuggerMainWindow::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// In picking mode, continuously refresh data
	if (SharedState.IsValid() && SharedState->IsPickingMode())
	{
		SharedState->RequestRefresh();
	}
}

TSharedRef<SWidget> SGASDebuggerMainWindow::BuildTopBar()
{
	return SNew(SBorder)
		.Padding(4.f)
		[
			SNew(SHorizontalBox)

			// Refresh button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("Update", "Update"))
				.OnClicked(this, &SGASDebuggerMainWindow::OnRefreshButtonClicked)
			]

			// World selector
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				BuildWorldSelector()
			]

			// Separator
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SSeparator)
				.Orientation(Orient_Vertical)
			]

			// Picking mode checkbox
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SCheckBox)
				.Padding(FMargin(4, 0))
				.IsChecked(this, &SGASDebuggerMainWindow::GetPickingModeCheckState)
				.OnCheckStateChanged(this, &SGASDebuggerMainWindow::HandlePickingModeChanged)
				[
					SNew(SBox)
					.MinDesiredWidth(150)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(this, &SGASDebuggerMainWindow::GetPickingModeText)
					]
				]
			]

			// Actor selector
			+ SHorizontalBox::Slot()
			.FillWidth(1.f)
			.Padding(2.f)
			[
				BuildActorSelector()
			]

			// Separator before new window button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4.f, 0.f)
			[
				SNew(SSeparator)
				.Orientation(Orient_Vertical)
			]

			// New window button
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("NewWindow", "+"))
				.ToolTipText(LOCTEXT("NewWindowTooltip", "Open a new GAS Debugger window"))
				.OnClicked(this, &SGASDebuggerMainWindow::OnNewWindowButtonClicked)
			]
		];
}

TSharedRef<SWidget> SGASDebuggerMainWindow::BuildWorldSelector()
{
	return SNew(SComboButton)
		.OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
		{
			FMenuBuilder MenuBuilder(true, nullptr);

			if (!GEngine)
			{
				return MenuBuilder.MakeWidget();
			}

			const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();

			for (const FWorldContext& Context : WorldList)
			{
				if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
				{
					FUIAction Action(FExecuteAction::CreateSP(this, &SGASDebuggerMainWindow::HandleWorldSelectionChanged, Context.ContextHandle));

					FText DisplayName;
					if (Context.RunAsDedicated)
					{
						DisplayName = LOCTEXT("Dedicated", "Dedicated Server");
					}
					else if (Context.WorldType == EWorldType::Game)
					{
						DisplayName = LOCTEXT("Client", "Client");
					}
					else
					{
						DisplayName = FText::Format(FText::FromString("{0} [{1}]"), LOCTEXT("Client", "Client"), FText::AsNumber(Context.PIEInstance));
					}

					MenuBuilder.AddMenuEntry(DisplayName, FText::GetEmpty(), FSlateIcon(), Action);
				}
			}

			return MenuBuilder.MakeWidget();
		})
		.VAlign(VAlign_Center)
		.ContentPadding(2)
		.ButtonContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("SelectWorld", "Select World Scene"))
			.Text(this, &SGASDebuggerMainWindow::GetWorldSelectorText)
		];
}

void SGASDebuggerMainWindow::HandleWorldSelectionChanged(FName InContextHandle)
{
	if (!GEngine)
	{
		return;
	}

	// Update display text
	const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldList)
	{
		if (Context.ContextHandle == InContextHandle)
		{
			if (Context.RunAsDedicated)
			{
				SelectedWorldText = LOCTEXT("Dedicated", "Dedicated Server");
			}
			else if (Context.WorldType == EWorldType::Game)
			{
				SelectedWorldText = LOCTEXT("Client", "Client");
			}
			else
			{
				SelectedWorldText = FText::Format(FText::FromString("{0} [{1}]"), LOCTEXT("Client", "Client"), FText::AsNumber(Context.PIEInstance));
			}
			break;
		}
	}

	if (SharedState.IsValid())
	{
		SharedState->SetSelectedWorld(InContextHandle);
	}
}

FText SGASDebuggerMainWindow::GetWorldSelectorText() const
{
	if (SelectedWorldText.IsEmpty())
	{
		// Try to get first available world
		if (GEngine)
		{
			const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();
			for (const FWorldContext& Context : WorldList)
			{
				if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
				{
					if (Context.RunAsDedicated)
					{
						return LOCTEXT("Dedicated", "Dedicated Server");
					}
					else if (Context.WorldType == EWorldType::Game)
					{
						return LOCTEXT("Client", "Client");
					}
					else
					{
						return FText::Format(FText::FromString("{0} [{1}]"), LOCTEXT("Client", "Client"), FText::AsNumber(Context.PIEInstance));
					}
				}
			}
		}
		return LOCTEXT("NoWorld", "No World");
	}
	return SelectedWorldText;
}

TSharedRef<SWidget> SGASDebuggerMainWindow::BuildActorSelector()
{
	return SNew(SComboButton)
		.OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
		{
			if (SharedState.IsValid())
			{
				SharedState->RefreshASCList();
			}

			FMenuBuilder MenuBuilder(true, nullptr);

			if (SharedState.IsValid())
			{
				for (const TWeakObjectPtr<UAbilitySystemComponent>& ASC : SharedState->GetCachedASCList())
				{
					if (!ASC.IsValid())
					{
						continue;
					}

					AActor* Actor = GetActorFromASC(ASC);
					if (!Actor)
					{
						continue;
					}

					FUIAction Action(FExecuteAction::CreateSP(this, &SGASDebuggerMainWindow::HandleActorSelectionChanged, ASC));
					FText ActorName = FText::FromName(Actor->GetFName());

					MenuBuilder.AddMenuEntry(ActorName, FText::GetEmpty(), FSlateIcon(), Action);
				}
			}

			return MenuBuilder.MakeWidget();
		})
		.VAlign(VAlign_Center)
		.ContentPadding(2)
		.ButtonContent()
		[
			SNew(STextBlock)
			.ToolTipText(LOCTEXT("SelectActor", "Select Actor with ASC"))
			.Text(this, &SGASDebuggerMainWindow::GetActorSelectorText)
		];
}

void SGASDebuggerMainWindow::HandleActorSelectionChanged(TWeakObjectPtr<UAbilitySystemComponent> InASC)
{
	if (SharedState.IsValid())
	{
		SharedState->SetSelectedASC(InASC);
	}
}

FText SGASDebuggerMainWindow::GetActorSelectorText() const
{
	if (SharedState.IsValid())
	{
		UAbilitySystemComponent* ASC = SharedState->GetSelectedASC();
		if (ASC)
		{
			AActor* Actor = GetActorFromASC(MakeWeakObjectPtr(ASC));
			if (Actor)
			{
				return FText::FromName(Actor->GetFName());
			}
		}
	}
	return LOCTEXT("NoActorSelected", "Select Actor");
}

ECheckBoxState SGASDebuggerMainWindow::GetPickingModeCheckState() const
{
	if (SharedState.IsValid())
	{
		return SharedState->IsPickingMode() ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void SGASDebuggerMainWindow::HandlePickingModeChanged(ECheckBoxState NewState)
{
	if (SharedState.IsValid())
	{
		SharedState->SetPickingMode(NewState == ECheckBoxState::Checked);
	}
}

FText SGASDebuggerMainWindow::GetPickingModeText() const
{
	if (SharedState.IsValid() && SharedState->IsPickingMode())
	{
		return LOCTEXT("TickModeOn", "Ticking... (Press END to stop)");
	}
	return LOCTEXT("TickModeOff", "Tick");
}

FReply SGASDebuggerMainWindow::OnRefreshButtonClicked()
{
	if (SharedState.IsValid())
	{
		SharedState->RequestRefresh();
	}
	return FReply::Handled();
}

FReply SGASDebuggerMainWindow::OnNewWindowButtonClicked()
{
	FGASDebuggerModule::Get().SpawnNewDebuggerWindow();
	return FReply::Handled();
}

AActor* SGASDebuggerMainWindow::GetActorFromASC(const TWeakObjectPtr<UAbilitySystemComponent>& InASC)
{
	if (!InASC.IsValid())
	{
		return nullptr;
	}
	if (AActor* AvatarActor = InASC->GetAvatarActor_Direct())
	{
		return AvatarActor;
	}
	if (AActor* OwnerActor = InASC->GetOwnerActor())
	{
		return OwnerActor;
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE
