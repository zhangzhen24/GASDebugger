// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/Tabs/SGASDebuggerAttributesTab.h"
#include "Widgets/TreeNodes/GASAttributeTreeNode.h"
#include "Widgets/Views/SHeaderRow.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/SBoxPanel.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"

#define LOCTEXT_NAMESPACE "SGASDebuggerAttributesTab"

FText SGASDebuggerAttributesTab::GetTabLabel()
{
	return LOCTEXT("TabLabel", "Attributes");
}

void SGASDebuggerAttributesTab::Construct(const FArguments& InArgs)
{
	SharedState = InArgs._SharedState;
	SubscribeToSharedState();

	ChildSlot
	[
		SNew(SVerticalBox)

		// Search box
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(4.f)
		[
			SNew(SSearchBox)
			.HintText(LOCTEXT("SearchHint", "Search attributes..."))
			.OnTextChanged(this, &SGASDebuggerAttributesTab::OnSearchTextChanged)
		]

		// Attribute tree view
		+ SVerticalBox::Slot()
		.FillHeight(1.f)
		[
			SAssignNew(AttributeTreeView, STreeView<TSharedRef<FGASAttributeNodeBase>>)
			.TreeItemsSource(&AttributeTreeRoot)
			.OnGenerateRow(this, &SGASDebuggerAttributesTab::OnGenerateRow)
			.OnGetChildren(this, &SGASDebuggerAttributesTab::OnGetChildren)
			.SelectionMode(ESelectionMode::Single)
			.HeaderRow
			(
				SNew(SHeaderRow)
				+ SHeaderRow::Column(GASAttributeColumns::Name)
				.DefaultLabel(LOCTEXT("AttributeName", "Attribute"))
				.FillWidth(0.4f)

				+ SHeaderRow::Column(GASAttributeColumns::BaseValue)
				.DefaultLabel(LOCTEXT("BaseValue", "Base Value"))
				.FillWidth(0.3f)

				+ SHeaderRow::Column(GASAttributeColumns::CurrentValue)
				.DefaultLabel(LOCTEXT("CurrentValue", "Current Value"))
				.FillWidth(0.3f)
			)
		]
	];

	RefreshAttributeTree();
}

void SGASDebuggerAttributesTab::OnSelectionChanged()
{
	RefreshAttributeTree();
}

void SGASDebuggerAttributesTab::OnRefreshRequested()
{
	RefreshAttributeTree();
}

void SGASDebuggerAttributesTab::RefreshAttributeTree()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !AttributeTreeView.IsValid())
	{
		return;
	}

	AttributeTreeRoot.Reset();

	for (UAttributeSet* Set : ASC->GetSpawnedAttributes())
	{
		if (!Set)
		{
			continue;
		}

		FName SetName = Set->GetClass()->GetFName();

		for (TFieldIterator<FProperty> It(Set->GetClass()); It; ++It)
		{
			FProperty* Property = *It;
			if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
			{
				if (StructProp->Struct == FGameplayAttributeData::StaticStruct())
				{
					// Apply search filter
					FString AttributeName = Property->GetName();
					if (!PassesFilter(AttributeName))
					{
						continue;
					}

					FGameplayAttributeData* DataPtr = StructProp->ContainerPtrToValuePtr<FGameplayAttributeData>(Set);
					if (DataPtr)
					{
						FGASAttributeInfo Info;
						Info.Attribute = FGameplayAttribute(Property);
						Info.BaseValue = DataPtr->GetBaseValue();
						Info.CurrentValue = DataPtr->GetCurrentValue();
						Info.AttributeSetName = SetName;

						AttributeTreeRoot.Add(FGASAttributeNode::Create(Info));
					}
				}
			}
		}
	}

	AttributeTreeView->RequestTreeRefresh();
}

TSharedRef<ITableRow> SGASDebuggerAttributesTab::OnGenerateRow(TSharedRef<FGASAttributeNodeBase> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SGASAttributeTreeItem, OwnerTable)
		.NodeInfo(InItem);
}

void SGASDebuggerAttributesTab::OnGetChildren(TSharedRef<FGASAttributeNodeBase> InItem, TArray<TSharedRef<FGASAttributeNodeBase>>& OutChildren)
{
	OutChildren = InItem->GetChildren();
}

void SGASDebuggerAttributesTab::OnSearchTextChanged(const FText& InText)
{
	SearchText = InText.ToString();
	RefreshAttributeTree();
}

bool SGASDebuggerAttributesTab::PassesFilter(const FString& AttributeName) const
{
	if (SearchText.IsEmpty())
	{
		return true;
	}

	// Case-insensitive fuzzy search
	return AttributeName.Contains(SearchText, ESearchCase::IgnoreCase);
}

#undef LOCTEXT_NAMESPACE
