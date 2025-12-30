// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Framework/Application/IInputProcessor.h"

class FGASDebuggerSharedState;
class UAbilitySystemComponent;

/**
 * Main window widget for GASDebugger.
 * Contains the top bar with World/Actor/Picking selectors.
 * The actual content panels are managed by FTabManager as separate tabs.
 */
class SGASDebuggerMainWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SGASDebuggerMainWindow) {}
		SLATE_ARGUMENT(TSharedPtr<FGASDebuggerSharedState>, SharedState)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);
	virtual ~SGASDebuggerMainWindow();

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

private:
	// === Top Bar ===
	TSharedRef<SWidget> BuildTopBar();
	TSharedRef<SWidget> BuildWorldSelector();
	TSharedRef<SWidget> BuildActorSelector();

	// === World Selection ===
	void HandleWorldSelectionChanged(FName InContextHandle);
	FText GetWorldSelectorText() const;

	// === Actor Selection ===
	void HandleActorSelectionChanged(TWeakObjectPtr<UAbilitySystemComponent> InASC);
	FText GetActorSelectorText() const;

	// === Picking Mode ===
	ECheckBoxState GetPickingModeCheckState() const;
	void HandlePickingModeChanged(ECheckBoxState NewState);
	FText GetPickingModeText() const;

	// === Refresh ===
	FReply OnRefreshButtonClicked();

	// === New Window ===
	FReply OnNewWindowButtonClicked();

	// === Helper ===
	static AActor* GetActorFromASC(const TWeakObjectPtr<UAbilitySystemComponent>& InASC);

private:
	TSharedPtr<FGASDebuggerSharedState> SharedState;
	TSharedPtr<class IInputProcessor> InputProcessor;

	// Cached display text
	FText SelectedWorldText;
};

/**
 * Input processor to handle END key for stopping picking mode
 */
class FGASDebuggerInputProcessor : public IInputProcessor
{
public:
	FGASDebuggerInputProcessor(TSharedPtr<FGASDebuggerSharedState> InSharedState);
	virtual ~FGASDebuggerInputProcessor();

private:
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual const TCHAR* GetDebugName() const override { return TEXT("GASDebuggerInputProcessor"); }
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override {}

private:
	TWeakPtr<FGASDebuggerSharedState> SharedStateWeak;
};
