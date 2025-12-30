// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

/** Base class for tag tree nodes */
class FGASTagNodeBase
{
public:
	virtual ~FGASTagNodeBase() {}

	/** Get tag name */
	virtual FName GetTagName() const = 0;

	/** Get display text */
	virtual FText GetDisplayText() const = 0;

	/** Get full tag path */
	virtual FString GetFullPath() const = 0;

	/** Check if this is a group node (not an actual tag) */
	virtual bool IsGroupNode() const { return false; }

	/** Add a child node */
	void AddChildNode(TSharedRef<FGASTagNodeBase> InChild);

	/** Get children */
	const TArray<TSharedRef<FGASTagNodeBase>>& GetChildren() const { return Children; }

	/** Find or create child node by name */
	TSharedRef<FGASTagNodeBase> FindOrCreateChild(const FName& ChildName);

	/** Check if node matches search filter */
	virtual bool MatchesFilter(const FString& FilterText) const;

protected:
	FGASTagNodeBase() {}

	TArray<TSharedRef<FGASTagNodeBase>> Children;
};

/** Group node (intermediate node in hierarchy, not an actual tag) */
class FGASTagGroupNode : public FGASTagNodeBase
{
public:
	static TSharedRef<FGASTagGroupNode> Create(const FName& InGroupName, const FString& InFullPath);

	virtual FName GetTagName() const override { return GroupName; }
	virtual FText GetDisplayText() const override;
	virtual FString GetFullPath() const override { return FullPath; }
	virtual bool IsGroupNode() const override { return true; }

public:
	explicit FGASTagGroupNode(const FName& InGroupName, const FString& InFullPath);

private:
	FName GroupName;
	FString FullPath;
};

/** Concrete tag node implementation */
class FGASTagNode : public FGASTagNodeBase
{
public:
	static TSharedRef<FGASTagNode> Create(const FGameplayTag& InTag);

	virtual FName GetTagName() const override { return Tag.GetTagName(); }
	virtual FText GetDisplayText() const override;
	virtual FString GetFullPath() const override { return Tag.ToString(); }

	const FGameplayTag& GetTag() const { return Tag; }

public:
	explicit FGASTagNode(const FGameplayTag& InTag);

private:
	FGameplayTag Tag;
};

