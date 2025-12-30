// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/Tabs/SGASDebuggerEffectsTab.h"
#include "Widgets/TreeNodes/GASEffectTreeNode.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "AbilitySystemComponent.h"
#include "ActiveGameplayEffectHandle.h"

#define LOCTEXT_NAMESPACE "SGASDebuggerEffectsTab"

FText SGASDebuggerEffectsTab::GetTabLabel()
{
	return LOCTEXT("TabLabel", "GameplayEffects");
}

void SGASDebuggerEffectsTab::Construct(const FArguments& InArgs)
{
	SharedState = InArgs._SharedState;
	SubscribeToSharedState();

	SAssignNew(EffectTreeView, STreeView<TSharedRef<FGASEffectNodeBase>>)
		.TreeItemsSource(&EffectTreeRoot)
		.OnGenerateRow(this, &SGASDebuggerEffectsTab::OnGenerateRow)
		.OnGetChildren(this, &SGASDebuggerEffectsTab::OnGetChildren)
		.SelectionMode(ESelectionMode::Single)
		.HeaderRow
		(
			SNew(SHeaderRow)
			+ SHeaderRow::Column(GASEffectColumns::Name)
			.DefaultLabel(LOCTEXT("EffectName", "Effect"))
			.FillWidth(0.25f)
			.MinSize(150.f)

			+ SHeaderRow::Column(GASEffectColumns::Duration)
			.DefaultLabel(LOCTEXT("Duration", "Duration"))
			.FillWidth(0.25f)

			+ SHeaderRow::Column(GASEffectColumns::Stack)
			.DefaultLabel(LOCTEXT("Stack", "Stack"))
			.FillWidth(0.1f)

			+ SHeaderRow::Column(GASEffectColumns::Level)
			.DefaultLabel(LOCTEXT("Level", "Level"))
			.FillWidth(0.1f)

			+ SHeaderRow::Column(GASEffectColumns::Prediction)
			.DefaultLabel(LOCTEXT("Prediction", "Prediction"))
			.FillWidth(0.15f)

			+ SHeaderRow::Column(GASEffectColumns::GrantedTags)
			.DefaultLabel(LOCTEXT("GrantedTags", "Granted Tags"))
			.FillWidth(0.15f)
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
				.OnClicked(this, &SGASDebuggerEffectsTab::OnExpandAllClicked)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.f)
			[
				SNew(SButton)
				.Text(LOCTEXT("CollapseAll", "Collapse All"))
				.OnClicked(this, &SGASDebuggerEffectsTab::OnCollapseAllClicked)
			]
		]
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			EffectTreeView.ToSharedRef()
		]
	];

	RefreshEffectTree();
}

void SGASDebuggerEffectsTab::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	// Always refresh to update progress bars in real-time
	if (SharedState.IsValid() && SharedState->IsPickingMode())
	{
		RefreshEffectTree();
	}
}

void SGASDebuggerEffectsTab::OnSelectionChanged()
{
	RefreshEffectTree();
}

void SGASDebuggerEffectsTab::OnRefreshRequested()
{
	RefreshEffectTree();
}

void SGASDebuggerEffectsTab::RefreshEffectTree()
{
	UAbilitySystemComponent* ASC = GetASC();
	UWorld* World = GetWorld();
	if (!ASC || !EffectTreeView.IsValid() || !World)
	{
		return;
	}

	EffectTreeRoot.Reset();

	const FActiveGameplayEffectsContainer& ActiveEffects = ASC->GetActiveGameplayEffects();
	for (const FActiveGameplayEffect& ActiveGE : &ActiveEffects)
	{
		EffectTreeRoot.Add(FGASEffectNode::Create(World, ActiveGE));

		if (bTreeExpanded && EffectTreeView.IsValid())
		{
			EffectTreeView->SetItemExpansion(EffectTreeRoot.Last(), true);
		}
	}

	EffectTreeView->RequestTreeRefresh();
}

TSharedRef<ITableRow> SGASDebuggerEffectsTab::OnGenerateRow(TSharedRef<FGASEffectNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASEffectTreeItem, OwnerTable)
		.NodeInfo(InItem);
}

void SGASDebuggerEffectsTab::OnGetChildren(TSharedRef<FGASEffectNodeBase> InItem, TArray<TSharedRef<FGASEffectNodeBase>>& OutChildren)
{
	OutChildren = InItem->GetChildren();
}

FReply SGASDebuggerEffectsTab::OnExpandAllClicked()
{
	bTreeExpanded = true;

	if (EffectTreeView.IsValid())
	{
		for (const TSharedRef<FGASEffectNodeBase>& Node : EffectTreeRoot)
		{
			EffectTreeView->SetItemExpansion(Node, true);
		}
	}

	return FReply::Handled();
}

FReply SGASDebuggerEffectsTab::OnCollapseAllClicked()
{
	bTreeExpanded = false;

	if (EffectTreeView.IsValid())
	{
		for (const TSharedRef<FGASEffectNodeBase>& Node : EffectTreeRoot)
		{
			EffectTreeView->SetItemExpansion(Node, false);
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
