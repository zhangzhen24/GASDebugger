// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Tabs/SGASDebuggerTabBase.h"
#include "Widgets/Views/STreeView.h"

class FGASAttributeNodeBase;

/**
 * Attributes tab for GASDebugger.
 * Displays attribute sets and their values in a tree view.
 */
class SGASDebuggerAttributesTab : public SGASDebuggerTabBase
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerAttributesTab) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static FName GetTabId() { return FName("GASDebugger_Attributes"); }
	static FText GetTabLabel();

protected:
	virtual void OnSelectionChanged() override;
	virtual void OnRefreshRequested() override;

private:
	void RefreshAttributeTree();
	TSharedRef<ITableRow> OnGenerateRow(TSharedRef<FGASAttributeNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(TSharedRef<FGASAttributeNodeBase> InItem, TArray<TSharedRef<FGASAttributeNodeBase>>& OutChildren);
	void OnSearchTextChanged(const FText& InText);
	bool PassesFilter(const FString& AttributeName) const;

	TSharedPtr<STreeView<TSharedRef<FGASAttributeNodeBase>>> AttributeTreeView;
	TArray<TSharedRef<FGASAttributeNodeBase>> AttributeTreeRoot;
	FString SearchText;
};
