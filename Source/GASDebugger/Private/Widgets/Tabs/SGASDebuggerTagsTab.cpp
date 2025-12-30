// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/Tabs/SGASDebuggerTagsTab.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/SBoxPanel.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"

#define LOCTEXT_NAMESPACE "SGASDebuggerTagsTab"

FText SGASDebuggerTagsTab::GetTabLabel()
{
	return LOCTEXT("TabLabel", "Tags");
}

void SGASDebuggerTagsTab::Construct(const FArguments& InArgs)
{
	SharedState = InArgs._SharedState;
	SubscribeToSharedState();

	ChildSlot
	[
		SNew(SSplitter)
		.Orientation(Orient_Vertical)
		+ SSplitter::Slot()
		.Value(0.7f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(2.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("OwnedTags", "Owned Tags"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
				.Padding(2.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(OwnedTagsBox, SVerticalBox)
					]
				]
			]
		]

		+ SSplitter::Slot()
		.Value(0.3f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Left)
			.Padding(2.f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BlockedTags", "Blocked Tags"))
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.f)
			[
				SNew(SBorder)
				.Padding(2.f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(BlockedTagsBox, SVerticalBox)
					]
				]
			]
		]
	];

	RefreshTagDisplay();
}

void SGASDebuggerTagsTab::OnSelectionChanged()
{
	// Clear cache when selection changes to force rebuild
	CachedOwnedTags.Reset();
	CachedBlockedTags.Reset();
	RefreshTagDisplay();
}

void SGASDebuggerTagsTab::OnRefreshRequested()
{
	RefreshTagDisplay();
}

void SGASDebuggerTagsTab::RefreshTagDisplay()
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC || !OwnedTagsBox.IsValid() || !BlockedTagsBox.IsValid())
	{
		return;
	}

	// Get current tags
	FGameplayTagContainer CurrentOwnedTags;
	ASC->GetOwnedGameplayTags(CurrentOwnedTags);

	FGameplayTagContainer CurrentBlockedTags;
	ASC->GetBlockedAbilityTags(CurrentBlockedTags);

	// Only rebuild if tags have changed
	bool bOwnedTagsChanged = (CurrentOwnedTags != CachedOwnedTags);
	bool bBlockedTagsChanged = (CurrentBlockedTags != CachedBlockedTags);

	if (bOwnedTagsChanged)
	{
		CachedOwnedTags = CurrentOwnedTags;
		OwnedTagsBox->ClearChildren();

		// Sort tags alphabetically
		TArray<FGameplayTag> SortedOwnedTags;
		CurrentOwnedTags.GetGameplayTagArray(SortedOwnedTags);
		SortedOwnedTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
		{
			return A.ToString() < B.ToString();
		});

		for (const FGameplayTag& GameplayTag : SortedOwnedTags)
		{
			OwnedTagsBox->AddSlot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SBorder)
				.Padding(FMargin(4.f, 2.f))
				.BorderBackgroundColor(FSlateColor(FLinearColor(0.1f, 0.4f, 0.1f, 1.f)))
				[
					SNew(STextBlock)
					.Text(FText::FromString(GameplayTag.ToString()))
				]
			];
		}
	}

	if (bBlockedTagsChanged)
	{
		CachedBlockedTags = CurrentBlockedTags;
		BlockedTagsBox->ClearChildren();

		// Sort tags alphabetically
		TArray<FGameplayTag> SortedBlockedTags;
		CurrentBlockedTags.GetGameplayTagArray(SortedBlockedTags);
		SortedBlockedTags.Sort([](const FGameplayTag& A, const FGameplayTag& B)
		{
			return A.ToString() < B.ToString();
		});

		for (const FGameplayTag& GameplayTag : SortedBlockedTags)
		{
			BlockedTagsBox->AddSlot()
			.AutoHeight()
			.Padding(2.f)
			[
				SNew(SBorder)
				.Padding(FMargin(4.f, 2.f))
				.BorderBackgroundColor(FSlateColor(FLinearColor(0.4f, 0.1f, 0.1f, 1.f)))
				[
					SNew(STextBlock)
					.Text(FText::FromString(GameplayTag.ToString()))
				]
			];
		}
	}
}

#undef LOCTEXT_NAMESPACE
