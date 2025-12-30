// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/TreeNodes/GASTagTreeNode.h"
#include "GameplayTagContainer.h"

#define LOCTEXT_NAMESPACE "GASTagTreeNode"

void FGASTagNodeBase::AddChildNode(TSharedRef<FGASTagNodeBase> InChild)
{
	Children.Add(InChild);
}

TSharedRef<FGASTagNodeBase> FGASTagNodeBase::FindOrCreateChild(const FName& ChildName)
{
	// Check if child already exists
	for (const TSharedRef<FGASTagNodeBase>& Child : Children)
	{
		if (Child->GetTagName() == ChildName)
		{
			return Child;
		}
	}

	// Create new group node
	FString NewPath = GetFullPath();
	if (!NewPath.IsEmpty())
	{
		NewPath += TEXT(".");
	}
	NewPath += ChildName.ToString();

	TSharedRef<FGASTagGroupNode> NewChild = FGASTagGroupNode::Create(ChildName, NewPath);
	Children.Add(NewChild);
	return NewChild;
}

bool FGASTagNodeBase::MatchesFilter(const FString& FilterText) const
{
	if (FilterText.IsEmpty())
	{
		return true;
	}

	FString FullPath = GetFullPath();
	FString TagName = GetTagName().ToString();
	
	// For group nodes, also check if any child matches
	if (IsGroupNode() && !Children.IsEmpty())
	{
		for (const TSharedRef<FGASTagNodeBase>& Child : Children)
		{
			if (Child->MatchesFilter(FilterText))
			{
				return true;
			}
		}
	}
	
	return FullPath.Contains(FilterText) || TagName.Contains(FilterText);
}

//////////////////////////////////////////////////////////////////////////
// FGASTagGroupNode

TSharedRef<FGASTagGroupNode> FGASTagGroupNode::Create(const FName& InGroupName, const FString& InFullPath)
{
	return MakeShared<FGASTagGroupNode>(InGroupName, InFullPath);
}

FGASTagGroupNode::FGASTagGroupNode(const FName& InGroupName, const FString& InFullPath)
	: GroupName(InGroupName)
	, FullPath(InFullPath)
{
}

FText FGASTagGroupNode::GetDisplayText() const
{
	return FText::FromName(GroupName);
}

//////////////////////////////////////////////////////////////////////////
// FGASTagNode

TSharedRef<FGASTagNode> FGASTagNode::Create(const FGameplayTag& InTag)
{
	return MakeShared<FGASTagNode>(InTag);
}

FGASTagNode::FGASTagNode(const FGameplayTag& InTag)
	: Tag(InTag)
{
}

FText FGASTagNode::GetDisplayText() const
{
	FString TagStr = Tag.ToString();
	FString DisplayName = Tag.GetTagName().ToString();
	
	// Extract the last part of the tag path as display name
	int32 LastDot = TagStr.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
	if (LastDot != INDEX_NONE)
	{
		DisplayName = TagStr.Mid(LastDot + 1);
	}

	return FText::FromString(DisplayName);
}

#undef LOCTEXT_NAMESPACE

