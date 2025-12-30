// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Core/GASDebuggerSharedState.h"

class UAbilitySystemComponent;

/**
 * Base class for all GASDebugger tabs.
 * Provides common functionality for subscribing to shared state changes.
 */
class SGASDebuggerTabBase : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerTabBase) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	virtual ~SGASDebuggerTabBase();

protected:
	/** Called when World or Actor selection changes */
	virtual void OnSelectionChanged() {}

	/** Called when refresh is explicitly requested */
	virtual void OnRefreshRequested() {}

	/** Get the currently selected ASC */
	UAbilitySystemComponent* GetASC() const;

	/** Get the currently selected World */
	UWorld* GetWorld() const;

	/** Subscribe to shared state delegates */
	void SubscribeToSharedState();

	/** Unsubscribe from shared state delegates */
	void UnsubscribeFromSharedState();

	TSharedPtr<FGASDebuggerSharedState> SharedState;
	FDelegateHandle SelectionChangedHandle;
	FDelegateHandle RefreshRequestedHandle;
};
