// Copyright Epic Games, Inc. All Rights Reserved.

#include "Widgets/TreeNodes/GASAttributeTreeNode.h"
#include "GASDebuggerTypes.h"
#include "AbilitySystemComponent.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/SExpanderArrow.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SNullWidget.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "GASAttributeTreeNode"

float FGASAttributeNodeBase::GetChangePercent() const
{
	float Base = GetBaseValue();
	if (FMath::IsNearlyZero(Base))
	{
		return 0.0f;
	}
	return (GetCurrentValue() - Base) / Base;
}

FLinearColor FGASAttributeNodeBase::GetChangeColor() const
{
	if (IsGroupNode() || IsModifierNode())
	{
		return FLinearColor::White;
	}

	float ChangePercent = GetChangePercent();
	if (FMath::IsNearlyZero(ChangePercent))
	{
		return FLinearColor::White;
	}

	return ChangePercent > 0.0f ? FLinearColor::Green : FLinearColor::Red;
}

void FGASAttributeNodeBase::AddChildNode(TSharedRef<FGASAttributeNodeBase> InChild)
{
	Children.Add(InChild);
}

//////////////////////////////////////////////////////////////////////////
// FGASAttributeSetNode

TSharedRef<FGASAttributeSetNode> FGASAttributeSetNode::Create(const FName& InAttributeSetName)
{
	return MakeShared<FGASAttributeSetNode>(InAttributeSetName);
}

FGASAttributeSetNode::FGASAttributeSetNode(const FName& InAttributeSetName)
	: AttributeSetName(InAttributeSetName)
{
}

FText FGASAttributeSetNode::GetDisplayText() const
{
	return FText::FromName(AttributeSetName);
}

//////////////////////////////////////////////////////////////////////////
// FGASAttributeNode

TSharedRef<FGASAttributeNode> FGASAttributeNode::Create(const FGASAttributeInfo& InAttributeInfo)
{
	return MakeShared<FGASAttributeNode>(InAttributeInfo);
}

FGASAttributeNode::FGASAttributeNode(const FGASAttributeInfo& InAttributeInfo)
	: AttributeInfo(InAttributeInfo)
{
}

FName FGASAttributeNode::GetDisplayName() const
{
	if (!AttributeInfo.Attribute.IsValid())
	{
		return NAME_None;
	}
	FString AttrNameStr = AttributeInfo.Attribute.GetName();
	return FName(*AttrNameStr);
}

FText FGASAttributeNode::GetDisplayText() const
{
	if (!AttributeInfo.Attribute.IsValid())
	{
		return FText::FromString(TEXT("(Invalid)"));
	}
	
	FString AttrNameStr = AttributeInfo.Attribute.GetName();
	return FText::FromString(AttrNameStr);
}

float FGASAttributeNode::GetChangePercent() const
{
	float Base = AttributeInfo.BaseValue;
	if (FMath::IsNearlyZero(Base))
	{
		return 0.0f;
	}
	return (AttributeInfo.CurrentValue - Base) / Base;
}

//////////////////////////////////////////////////////////////////////////
// SGASAttributeTreeItem

void SGASAttributeTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	NodeInfo = InArgs._NodeInfo;
	SetPadding(0);

	check(NodeInfo.IsValid());

	// Cache data
	bIsGroupNode = NodeInfo->IsGroupNode();
	CachedDisplayText = NodeInfo->GetDisplayText();
	
	if (!bIsGroupNode)
	{
		CachedBaseValue = NodeInfo->GetBaseValue();
		CachedCurrentValue = NodeInfo->GetCurrentValue();
		CachedChangeColor = NodeInfo->GetChangeColor();
		bHasModifier = !FMath::IsNearlyEqual(CachedBaseValue, CachedCurrentValue);
	}

	SMultiColumnTableRow<TSharedRef<FGASAttributeNodeBase>>::Construct(
		SMultiColumnTableRow<TSharedRef<FGASAttributeNodeBase>>::FArguments().Padding(0),
		InOwnerTableView);
}

TSharedRef<SWidget> SGASAttributeTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == GASAttributeColumns::Name)
	{
		if (bIsGroupNode)
		{
			// AttributeSet group node
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(16)
					.ShouldDrawWires(true)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(4, 2)
				[
					SNew(STextBlock)
					.Text(CachedDisplayText)
					.Font(FAppStyle::GetFontStyle("Bold"))
				];
		}
		else
		{
			// Attribute node
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SExpanderArrow, SharedThis(this))
					.IndentAmount(16)
					.ShouldDrawWires(true)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				.Padding(4, 2)
				[
					SNew(STextBlock)
					.Text(CachedDisplayText)
				];
		}
	}
	else if (ColumnName == GASAttributeColumns::BaseValue)
	{
		if (bIsGroupNode)
		{
			return SNullWidget::NullWidget;
		}

		return SNew(SBox)
			.HAlign(HAlign_Left)
			.MinDesiredWidth(100.0f)
			.VAlign(VAlign_Center)
			.Padding(4, 2)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("BaseFormat", "Base: {0}"),
					FText::AsNumber(CachedBaseValue, &FNumberFormattingOptions::DefaultWithGrouping())))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			];
	}
	else if (ColumnName == GASAttributeColumns::CurrentValue)
	{
		if (bIsGroupNode)
		{
			return SNullWidget::NullWidget;
		}

		return SNew(SBox)
			.HAlign(HAlign_Left)
			.MinDesiredWidth(120.0f)
			.VAlign(VAlign_Center)
			.Padding(4, 2)
			[
				SNew(STextBlock)
				.Text(FText::Format(LOCTEXT("CurrentFormat", "Current: {0}"),
					FText::AsNumber(CachedCurrentValue, &FNumberFormattingOptions::DefaultWithGrouping())))
				.ColorAndOpacity(FSlateColor(bHasModifier ? CachedChangeColor : FLinearColor::White))
			];
	}

	return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE

