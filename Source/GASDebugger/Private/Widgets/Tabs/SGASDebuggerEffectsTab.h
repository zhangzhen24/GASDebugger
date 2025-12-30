// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Tabs/SGASDebuggerTabBase.h"
#include "Widgets/Views/STreeView.h"

class FGASEffectNodeBase;

/**
 * GameplayEffects tab for GASDebugger.
 * Displays active gameplay effects with progress bars for duration.
 */
class SGASDebuggerEffectsTab : public SGASDebuggerTabBase
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerEffectsTab) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	static FName GetTabId() { return FName("GASDebugger_Effects"); }
	static FText GetTabLabel();

protected:
	virtual void OnSelectionChanged() override;
	virtual void OnRefreshRequested() override;

private:
	void RefreshEffectTree();
	TSharedRef<ITableRow> OnGenerateRow(TSharedRef<FGASEffectNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedRef<FGASEffectNodeBase> InItem, TArray<TSharedRef<FGASEffectNodeBase>>& OutChildren);

	FReply OnExpandAllClicked();
	FReply OnCollapseAllClicked();

	TSharedPtr<STreeView<TSharedRef<FGASEffectNodeBase>>> EffectTreeView;
	TArray<TSharedRef<FGASEffectNodeBase>> EffectTreeRoot;
	bool bTreeExpanded = false;
};
