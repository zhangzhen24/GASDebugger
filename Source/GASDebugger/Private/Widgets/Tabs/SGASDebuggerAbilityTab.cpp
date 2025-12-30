// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/Tabs/SGASDebuggerAbilityTab.h"
#include "Widgets/TreeNodes/GASAbilityTreeNode.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"

#define LOCTEXT_NAMESPACE "SGASDebuggerAbilityTab"

FText SGASDebuggerAbilityTab::GetTabLabel()
{
	return LOCTEXT("TabLabel", "Abilities");
}

void SGASDebuggerAbilityTab::Construct(const FArguments& InArgs)
{
	SharedState = InArgs._SharedState;
	SubscribeToSharedState();

	SAssignNew(AbilityTreeView, STreeView<TSharedRef<FGASAbilityNodeBase>>)
		.TreeItemsSource(&AbilityTreeRoot)
		.OnGenerateRow(this, &SGASDebuggerAbilityTab::OnGenerateRow)
		.OnGetChildren(this, &SGASDebuggerAbilityTab::OnGetChildren)
		.SelectionMode(ESelectionMode::Single)
		.HeaderRow
		(
			SNew(SHeaderRow)
			+ SHeaderRow::Column(GASAbilityColumns::Name)
			.DefaultLabel(LOCTEXT("AbilityName", "Ability"))
			.FillWidth(0.4f)
			.MinSize(200.f)

			+ SHeaderRow::Column(GASAbilityColumns::State)
			.DefaultLabel(LOCTEXT("State", "State"))
			.FillWidth(0.2f)

			+ SHeaderRow::Column(GASAbilityColumns::IsActive)
			.DefaultLabel(LOCTEXT("Active", "Active"))
			.FixedWidth(60.f)

			+ SHeaderRow::Column(GASAbilityColumns::Triggers)
			.DefaultLabel(LOCTEXT("Triggers", "Triggers"))
			.FillWidth(0.2f)
		);

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("ExpandAll", "Expand All"))
				.OnClicked(this, &SGASDebuggerAbilityTab::OnExpandAllClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("CollapseAll", "Collapse All"))
				.OnClicked(this, &SGASDebuggerAbilityTab::OnCollapseAllClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(10.f, 2.f, 2.f, 2.f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SGASDebuggerAbilityTab::GetAbilityFilterState, EGASAbilityFilterState::Active)
				.OnCheckStateChanged(this, &SGASDebuggerAbilityTab::HandleAbilityFilterChanged, EGASAbilityFilterState::Active)
				[
					SNew(STextBlock).Text(LOCTEXT("FilterActive", "Active"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SGASDebuggerAbilityTab::GetAbilityFilterState, EGASAbilityFilterState::Blocked)
				.OnCheckStateChanged(this, &SGASDebuggerAbilityTab::HandleAbilityFilterChanged, EGASAbilityFilterState::Blocked)
				[
					SNew(STextBlock).Text(LOCTEXT("FilterBlocked", "Blocked"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SCheckBox)
				.IsChecked(this, &SGASDebuggerAbilityTab::GetAbilityFilterState, EGASAbilityFilterState::Inactive)
				.OnCheckStateChanged(this, &SGASDebuggerAbilityTab::HandleAbilityFilterChanged, EGASAbilityFilterState::Inactive)
				[
					SNew(STextBlock).Text(LOCTEXT("FilterInactive", "Inactive"))
				]
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			AbilityTreeView.ToSharedRef()
		]
	];

	RefreshAbilityTree();
}

void SGASDebuggerAbilityTab::OnSelectionChanged()
{
	RefreshAbilityTree();
}

void SGASDebuggerAbilityTab::OnRefreshRequested()
{
	RefreshAbilityTree();
}

void SGASDebuggerAbilityTab::RefreshAbilityTree()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !AbilityTreeView.IsValid())
	{
		return;
	}

	AbilityTreeRoot.Reset();

	for (const FGameplayAbilitySpec& Spec : ASC->GetActivatableAbilities())
	{
		TSharedRef<FGASAbilityNode> Node = FGASAbilityNode::Create(ASC, Spec);

		// Apply filter
		EGASAbilityState State = Node->GetState();
		bool bShouldShow = false;

		if ((AbilityFilterState & EGASAbilityFilterState::Active) && State == EGASAbilityState::Active)
		{
			bShouldShow = true;
		}
		else if ((AbilityFilterState & EGASAbilityFilterState::Blocked) &&
			(State == EGASAbilityState::InputBlocked || State == EGASAbilityState::TagBlocked || State == EGASAbilityState::Cooldown))
		{
			bShouldShow = true;
		}
		else if ((AbilityFilterState & EGASAbilityFilterState::Inactive) &&
			(State == EGASAbilityState::Ready || State == EGASAbilityState::CantActivate))
		{
			bShouldShow = true;
		}

		if (bShouldShow)
		{
			AbilityTreeRoot.Add(Node);

			if (bTreeExpanded && AbilityTreeView.IsValid())
			{
				AbilityTreeView->SetItemExpansion(Node, true);
			}
		}
	}

	AbilityTreeView->RequestTreeRefresh();
}

TSharedRef<ITableRow> SGASDebuggerAbilityTab::OnGenerateRow(TSharedRef<FGASAbilityNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASAbilityTreeItem, OwnerTable)
		.NodeInfo(InItem);
}

void SGASDebuggerAbilityTab::OnGetChildren(TSharedRef<FGASAbilityNodeBase> InItem, TArray<TSharedRef<FGASAbilityNodeBase>>& OutChildren)
{
	OutChildren = InItem->GetChildren();
}

void SGASDebuggerAbilityTab::HandleAbilityFilterChanged(ECheckBoxState NewState, EGASAbilityFilterState FilterFlag)
{
	if (NewState == ECheckBoxState::Checked)
	{
		AbilityFilterState |= FilterFlag;
	}
	else
	{
		AbilityFilterState &= ~FilterFlag;
	}
	RefreshAbilityTree();
}

ECheckBoxState SGASDebuggerAbilityTab::GetAbilityFilterState(EGASAbilityFilterState FilterFlag) const
{
	return (AbilityFilterState & FilterFlag) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

FReply SGASDebuggerAbilityTab::OnExpandAllClicked()
{
	bTreeExpanded = true;

	if (AbilityTreeView.IsValid())
	{
		for (const TSharedRef<FGASAbilityNodeBase>& Node : AbilityTreeRoot)
		{
			AbilityTreeView->SetItemExpansion(Node, true);
		}
	}

	return FReply::Handled();
}

FReply SGASDebuggerAbilityTab::OnCollapseAllClicked()
{
	bTreeExpanded = false;

	if (AbilityTreeView.IsValid())
	{
		for (const TSharedRef<FGASAbilityNodeBase>& Node : AbilityTreeRoot)
		{
			AbilityTreeView->SetItemExpansion(Node, false);
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
