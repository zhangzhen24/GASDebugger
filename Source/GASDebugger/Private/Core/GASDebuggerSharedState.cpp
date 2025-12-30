// Copyright Qiu, Inc. All Rights Reserved.

#include "Core/GASDebuggerSharedState.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Actor.h"
#include "EngineUtils.h"

FGASDebuggerSharedState::FGASDebuggerSharedState()
{
}

UWorld* FGASDebuggerSharedState::GetSelectedWorld() const
{
	if (!GEngine)
	{
		return nullptr;
	}

	const TIndirectArray<FWorldContext>& WorldList = GEngine->GetWorldContexts();
	for (const FWorldContext& Context : WorldList)
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			if (Context.ContextHandle == SelectedWorldContextHandle)
			{
				return Context.World();
			}
		}
	}

	// Return first valid world if no specific selection
	for (const FWorldContext& Context : WorldList)
	{
		if (Context.WorldType == EWorldType::PIE || Context.WorldType == EWorldType::Game)
		{
			return Context.World();
		}
	}

	return nullptr;
}

void FGASDebuggerSharedState::SetSelectedWorld(FName InWorldContextHandle)
{
	if (SelectedWorldContextHandle != InWorldContextHandle)
	{
		SelectedWorldContextHandle = InWorldContextHandle;
		SelectedASC.Reset();
		RefreshASCList();
		OnSelectionChanged.Broadcast();
	}
}

UAbilitySystemComponent* FGASDebuggerSharedState::GetSelectedASC() const
{
	return SelectedASC.Get();
}

void FGASDebuggerSharedState::SetSelectedASC(TWeakObjectPtr<UAbilitySystemComponent> InASC)
{
	if (SelectedASC != InASC)
	{
		SelectedASC = InASC;
		OnSelectionChanged.Broadcast();
	}
}

void FGASDebuggerSharedState::SetPickingMode(bool bEnabled)
{
	bPickingMode = bEnabled;
}

void FGASDebuggerSharedState::RequestRefresh()
{
	RefreshASCList();
	OnRefreshRequested.Broadcast();
}

void FGASDebuggerSharedState::RefreshASCList()
{
	CachedASCList.Reset();

	UWorld* World = GetSelectedWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor)
		{
			if (UAbilitySystemComponent* ASC = Actor->FindComponentByClass<UAbilitySystemComponent>())
			{
				CachedASCList.Add(ASC);
			}
		}
	}

	// Auto-select first ASC if no current selection
	if (!SelectedASC.IsValid() && CachedASCList.Num() > 0)
	{
		SelectedASC = CachedASCList[0];
	}
}
