// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Tabs/SGASDebuggerTabBase.h"
#include "Widgets/Views/STreeView.h"
#include "GASDebuggerTypes.h"

class FGASAbilityNodeBase;

/**
 * Ability tab for GASDebugger.
 * Displays abilities with state filters.
 */
class SGASDebuggerAbilityTab : public SGASDebuggerTabBase
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerAbilityTab) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static FName GetTabId() { return FName("GASDebugger_Ability"); }
	static FText GetTabLabel();

protected:
	virtual void OnSelectionChanged() override;
	virtual void OnRefreshRequested() override;

private:
	void RefreshAbilityTree();
	TSharedRef<ITableRow> OnGenerateRow(TSharedRef<FGASAbilityNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedRef<FGASAbilityNodeBase> InItem, TArray<TSharedRef<FGASAbilityNodeBase>>& OutChildren);

	void HandleAbilityFilterChanged(ECheckBoxState NewState, EGASAbilityFilterState FilterFlag);
	ECheckBoxState GetAbilityFilterState(EGASAbilityFilterState FilterFlag) const;

	FReply OnExpandAllClicked();
	FReply OnCollapseAllClicked();

	TSharedPtr<STreeView<TSharedRef<FGASAbilityNodeBase>>> AbilityTreeView;
	TArray<TSharedRef<FGASAbilityNodeBase>> AbilityTreeRoot;
	uint8 AbilityFilterState = EGASAbilityFilterState::Active | EGASAbilityFilterState::Blocked | EGASAbilityFilterState::Inactive;
	bool bTreeExpanded = false;
};
