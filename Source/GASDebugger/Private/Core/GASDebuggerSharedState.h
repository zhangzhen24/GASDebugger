// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"

/**
 * Shared state class for GASDebugger tabs.
 * Manages World/Actor selection and provides delegates for state changes.
 */
class FGASDebuggerSharedState : public TSharedFromThis<FGASDebuggerSharedState>
{
public:
	DECLARE_MULTICAST_DELEGATE(FOnSelectionChanged);
	DECLARE_MULTICAST_DELEGATE(FOnRefreshRequested);

	FOnSelectionChanged OnSelectionChanged;
	FOnRefreshRequested OnRefreshRequested;

	FGASDebuggerSharedState();

	// World selection
	UWorld* GetSelectedWorld() const;
	FName GetSelectedWorldContextHandle() const { return SelectedWorldContextHandle; }
	void SetSelectedWorld(FName InWorldContextHandle);

	// ASC selection
	UAbilitySystemComponent* GetSelectedASC() const;
	void SetSelectedASC(TWeakObjectPtr<UAbilitySystemComponent> InASC);

	// Picking mode
	bool IsPickingMode() const { return bPickingMode; }
	void SetPickingMode(bool bEnabled);

	// Refresh
	void RequestRefresh();

	// ASC list management
	void RefreshASCList();
	const TArray<TWeakObjectPtr<UAbilitySystemComponent>>& GetCachedASCList() const { return CachedASCList; }

private:
	FName SelectedWorldContextHandle;
	TWeakObjectPtr<UAbilitySystemComponent> SelectedASC;
	bool bPickingMode = true;
	TArray<TWeakObjectPtr<UAbilitySystemComponent>> CachedASCList;
};
