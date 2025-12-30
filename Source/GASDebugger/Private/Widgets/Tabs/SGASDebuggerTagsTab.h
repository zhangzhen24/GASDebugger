// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Tabs/SGASDebuggerTabBase.h"
#include "GameplayTagContainer.h"

class SVerticalBox;

/**
 * Tags tab for GASDebugger.
 * Displays Owned Tags and Blocked Tags with a draggable splitter.
 */
class SGASDebuggerTagsTab : public SGASDebuggerTabBase
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerTagsTab) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	static FName GetTabId() { return FName("GASDebugger_Tags"); }
	static FText GetTabLabel();

protected:
	virtual void OnSelectionChanged() override;
	virtual void OnRefreshRequested() override;

private:
	void RefreshTagDisplay();

	TSharedPtr<SVerticalBox> OwnedTagsBox;
	TSharedPtr<SVerticalBox> BlockedTagsBox;

	// Cached tags to avoid unnecessary UI rebuilds
	FGameplayTagContainer CachedOwnedTags;
	FGameplayTagContainer CachedBlockedTags;
};
