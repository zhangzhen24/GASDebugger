// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GASDebuggerTypes.h"
#include "AbilitySystemComponent.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

/** Column names for the attribute tree */
namespace GASAttributeColumns
{
	static const FName Name(TEXT("Name"));
	static const FName BaseValue(TEXT("BaseValue"));
	static const FName CurrentValue(TEXT("CurrentValue"));
}

/** Base class for attribute tree nodes */
class FGASAttributeNodeBase
{
public:
	virtual ~FGASAttributeNodeBase() {}

	/** Get display name */
	virtual FName GetDisplayName() const = 0;

	/** Get display text */
	virtual FText GetDisplayText() const = 0;

	/** Get base value (for attribute nodes) */
	virtual float GetBaseValue() const { return 0.0f; }

	/** Get current value (for attribute nodes) */
	virtual float GetCurrentValue() const { return 0.0f; }

	/** Get change percent (for attribute nodes) */
	virtual float GetChangePercent() const;

	/** Get change color based on value change */
	virtual FLinearColor GetChangeColor() const;

	/** Check if this is a group node (AttributeSet) */
	virtual bool IsGroupNode() const { return false; }

	/** Check if this is a modifier node */
	virtual bool IsModifierNode() const { return false; }

	/** Add a child node */
	void AddChildNode(TSharedRef<FGASAttributeNodeBase> InChild);

	/** Get children */
	const TArray<TSharedRef<FGASAttributeNodeBase>>& GetChildren() const { return Children; }

protected:
	FGASAttributeNodeBase() {}

	TArray<TSharedRef<FGASAttributeNodeBase>> Children;
};

/** Group node representing an AttributeSet */
class FGASAttributeSetNode : public FGASAttributeNodeBase
{
public:
	static TSharedRef<FGASAttributeSetNode> Create(const FName& InAttributeSetName);

	virtual FName GetDisplayName() const override { return AttributeSetName; }
	virtual FText GetDisplayText() const override;
	virtual bool IsGroupNode() const override { return true; }

public:
	explicit FGASAttributeSetNode(const FName& InAttributeSetName);

private:
	FName AttributeSetName;
};

/** Attribute node representing a single attribute */
class FGASAttributeNode : public FGASAttributeNodeBase
{
public:
	static TSharedRef<FGASAttributeNode> Create(const FGASAttributeInfo& InAttributeInfo);

	virtual FName GetDisplayName() const override;
	virtual FText GetDisplayText() const override;
	virtual float GetBaseValue() const override { return AttributeInfo.BaseValue; }
	virtual float GetCurrentValue() const override { return AttributeInfo.CurrentValue; }
	virtual float GetChangePercent() const override;

	const FGASAttributeInfo& GetAttributeInfo() const { return AttributeInfo; }

public:
	explicit FGASAttributeNode(const FGASAttributeInfo& InAttributeInfo);

private:
	FGASAttributeInfo AttributeInfo;
};

/** Tree row widget for attributes */
class SGASAttributeTreeItem : public SMultiColumnTableRow<TSharedRef<FGASAttributeNodeBase>>
{
public:
	SLATE_BEGIN_ARGS(SGASAttributeTreeItem)
		: _NodeInfo()
	{}
		SLATE_ARGUMENT(TSharedPtr<FGASAttributeNodeBase>, NodeInfo)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FGASAttributeNodeBase> NodeInfo;

	// Cached data
	FText CachedDisplayText;
	float CachedBaseValue = 0.0f;
	float CachedCurrentValue = 0.0f;
	FLinearColor CachedChangeColor = FLinearColor::White;
	bool bIsGroupNode = false;
	bool bHasModifier = false;
};

