// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/Tabs/SGASDebuggerTabBase.h"
#include "AbilitySystemComponent.h"

SGASDebuggerTabBase::~SGASDebuggerTabBase()
{
	UnsubscribeFromSharedState();
}

UAbilitySystemComponent* SGASDebuggerTabBase::GetASC() const
{
	if (SharedState.IsValid())
	{
		return SharedState->GetSelectedASC();
	}
	return nullptr;
}

UWorld* SGASDebuggerTabBase::GetWorld() const
{
	if (SharedState.IsValid())
	{
		return SharedState->GetSelectedWorld();
	}
	return nullptr;
}

void SGASDebuggerTabBase::SubscribeToSharedState()
{
	if (SharedState.IsValid())
	{
		SelectionChangedHandle = SharedState->OnSelectionChanged.AddSP(
			this, &SGASDebuggerTabBase::OnSelectionChanged);
		RefreshRequestedHandle = SharedState->OnRefreshRequested.AddSP(
			this, &SGASDebuggerTabBase::OnRefreshRequested);
	}
}

void SGASDebuggerTabBase::UnsubscribeFromSharedState()
{
	if (SharedState.IsValid())
	{
		SharedState->OnSelectionChanged.Remove(SelectionChangedHandle);
		SharedState->OnRefreshRequested.Remove(RefreshRequestedHandle);
	}
}
