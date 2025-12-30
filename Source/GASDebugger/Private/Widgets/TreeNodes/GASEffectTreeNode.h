// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "ActiveGameplayEffectHandle.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

class UAbilitySystemComponent;
class UWorld;

/** Column names for the effect tree */
namespace GASEffectColumns
{
	static const FName Name(TEXT("Name"));
	static const FName Duration(TEXT("Duration"));
	static const FName Stack(TEXT("Stack"));
	static const FName Level(TEXT("Level"));
	static const FName Prediction(TEXT("Prediction"));
	static const FName GrantedTags(TEXT("GrantedTags"));
}

/** Base class for effect tree nodes */
class FGASEffectNodeBase : public TSharedFromThis<FGASEffectNodeBase>
{
public:
	virtual ~FGASEffectNodeBase() {}

	/** Get the display name */
	virtual FName GetName() const = 0;

	/** Get duration text */
	virtual FText GetDurationText() const = 0;

	/** Get duration progress (0.0 to 1.0, or -1.0 for infinite) */
	virtual float GetDurationProgress() const = 0;

	/** Get stack text */
	virtual FText GetStackText() const = 0;

	/** Get level string */
	virtual FName GetLevelStr() const = 0;

	/** Get prediction text */
	virtual FText GetPredictionText() const = 0;

	/** Get granted tags as name */
	virtual FName GetGrantedTagsName() const = 0;

	/** Is this a modifier node (child) */
	virtual bool IsModifierNode() const = 0;

	/** Add a child node */
	void AddChildNode(TSharedRef<FGASEffectNodeBase> InChild);

	/** Get children */
	const TArray<TSharedRef<FGASEffectNodeBase>>& GetChildren() const { return Children; }

protected:
	FGASEffectNodeBase() {}

	TArray<TSharedRef<FGASEffectNodeBase>> Children;
};

/** Concrete effect node implementation */
class FGASEffectNode : public FGASEffectNodeBase
{
public:
	virtual ~FGASEffectNode() {}

	/** Create an effect node */
	static TSharedRef<FGASEffectNode> Create(
		const UWorld* InWorld,
		const FActiveGameplayEffect& InEffect);

	/** Create a modifier node */
	static TSharedRef<FGASEffectNode> CreateModifier(
		const FModifierSpec* InModSpec,
		const FGameplayModifierInfo* InModInfo);

	// FGASEffectNodeBase interface
	virtual FName GetName() const override;
	virtual FText GetDurationText() const override;
	virtual float GetDurationProgress() const override;
	virtual FText GetStackText() const override;
	virtual FName GetLevelStr() const override;
	virtual FText GetPredictionText() const override;
	virtual FName GetGrantedTagsName() const override;
	virtual bool IsModifierNode() const override { return ModSpec != nullptr; }

private:
	explicit FGASEffectNode(
		const UWorld* InWorld,
		const FActiveGameplayEffect& InEffect);

	explicit FGASEffectNode(
		const FModifierSpec* InModSpec,
		const FGameplayModifierInfo* InModInfo);

	/** Populate modifiers as children */
	void PopulateModifiers();

private:
	const UWorld* World = nullptr;
	FActiveGameplayEffect GameplayEffect;
	const FModifierSpec* ModSpec = nullptr;
	const FGameplayModifierInfo* ModInfo = nullptr;
};

/** Tree row widget for effects */
class SGASEffectTreeItem : public SMultiColumnTableRow<TSharedRef<FGASEffectNodeBase>>
{
public:
	SLATE_BEGIN_ARGS(SGASEffectTreeItem)
		: _NodeInfo()
	{}
		SLATE_ARGUMENT(TSharedPtr<FGASEffectNodeBase>, NodeInfo)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:
	TSharedPtr<FGASEffectNodeBase> NodeInfo;

	// Cached data
	FName CachedName;
	FText CachedDurationText;
	float CachedDurationProgress = -1.0f;
	FText CachedStackText;
	FName CachedLevelStr;
	FText CachedPredictionText;
	FName CachedGrantedTags;
	bool bIsModifier = false;
};
