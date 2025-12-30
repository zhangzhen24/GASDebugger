// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/TreeNodes/GASEffectTreeNode.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "SlateOptMacros.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Styling/AppStyle.h"

#define LOCTEXT_NAMESPACE "GASEffectTreeNode"

//////////////////////////////////////////////////////////////////////////
// FGASEffectNodeBase

void FGASEffectNodeBase::AddChildNode(TSharedRef<FGASEffectNodeBase> InChild)
{
	Children.Add(MoveTemp(InChild));
}

//////////////////////////////////////////////////////////////////////////
// FGASEffectNode

TSharedRef<FGASEffectNode> FGASEffectNode::Create(
	const UWorld* InWorld,
	const FActiveGameplayEffect& InEffect)
{
	return MakeShareable(new FGASEffectNode(InWorld, InEffect));
}

TSharedRef<FGASEffectNode> FGASEffectNode::CreateModifier(
	const FModifierSpec* InModSpec,
	const FGameplayModifierInfo* InModInfo)
{
	return MakeShareable(new FGASEffectNode(InModSpec, InModInfo));
}

FGASEffectNode::FGASEffectNode(
	const UWorld* InWorld,
	const FActiveGameplayEffect& InEffect)
	: World(InWorld), GameplayEffect(InEffect), ModSpec(nullptr), ModInfo(nullptr)
{
	PopulateModifiers();
}

FGASEffectNode::FGASEffectNode(
	const FModifierSpec* InModSpec,
	const FGameplayModifierInfo* InModInfo)
	: World(nullptr), ModSpec(InModSpec), ModInfo(InModInfo)
{
}

FName FGASEffectNode::GetName() const
{
	if (ModSpec && ModInfo)
	{
		return *ModInfo->Attribute.GetName();
	}

	// Get clean class name instead of CDO name (which has "Default__" prefix)
	if (const UGameplayEffect* Def = GameplayEffect.Spec.Def)
	{
		FString ClassName = Def->GetClass()->GetName();
		// Remove "_C" suffix for Blueprint classes
		if (ClassName.EndsWith(TEXT("_C")))
		{
			ClassName.LeftChopInline(2);
		}
		return *ClassName;
	}

	return NAME_None;
}

FText FGASEffectNode::GetDurationText() const
{
	// For modifier nodes, show the modifier info
	if (ModSpec && ModInfo)
	{
		UEnum* OpEnum = StaticEnum<EGameplayModOp::Type>();
		FString ModifierOpStr = OpEnum->GetNameStringByValue(static_cast<int64>(ModInfo->ModifierOp));
		return FText::Format(
			LOCTEXT("ModifierInfo", "Mod: {0}, Value: {1}"),
			FText::FromString(ModifierOpStr),
			FText::AsNumber(ModSpec->GetEvaluatedMagnitude()));
	}

	if (!World)
	{
		return FText::GetEmpty();
	}

	FNumberFormattingOptions NumberFormat;
	NumberFormat.MaximumFractionalDigits = 2;

	float Duration = GameplayEffect.GetDuration();
	if (Duration > 0.f)
	{
		float TimeRemaining = GameplayEffect.GetTimeRemaining(World->GetTimeSeconds());
		return FText::Format(
			LOCTEXT("DurationFormat", "Duration: {0}, Remaining: {1}"),
			FText::AsNumber(Duration, &NumberFormat),
			FText::AsNumber(TimeRemaining, &NumberFormat));
	}

	return LOCTEXT("InfiniteDuration", "Infinite Duration");
}

float FGASEffectNode::GetDurationProgress() const
{
	if (ModSpec || !World)
	{
		return -1.0f; // Not applicable
	}

	float Duration = GameplayEffect.GetDuration();
	if (Duration > 0.f)
	{
		float TimeRemaining = GameplayEffect.GetTimeRemaining(World->GetTimeSeconds());
		return FMath::Clamp(TimeRemaining / Duration, 0.0f, 1.0f);
	}

	return -1.0f; // Infinite duration
}

FText FGASEffectNode::GetStackText() const
{
	if (!World || ModSpec)
	{
		return FText::GetEmpty();
	}

	int32 StackCount = GameplayEffect.Spec.GetStackCount();
	if (StackCount > 1)
	{
		if (GameplayEffect.Spec.Def)
		{
			// Suppress deprecation warning - StackingType will be made private in future versions
			PRAGMA_DISABLE_DEPRECATION_WARNINGS
			bool bAggregateBySource = GameplayEffect.Spec.Def->StackingType == EGameplayEffectStackingType::AggregateBySource;
			PRAGMA_ENABLE_DEPRECATION_WARNINGS

			if (bAggregateBySource)
			{
				FString SourceName;
				if (GameplayEffect.Spec.GetContext().GetInstigatorAbilitySystemComponent())
				{
					AActor* SourceActor = GameplayEffect.Spec.GetContext().GetInstigatorAbilitySystemComponent()->GetAvatarActor_Direct();
					SourceName = GetNameSafe(SourceActor);
				}
				return FText::Format(
					LOCTEXT("StackWithSource", "Stacks: {0}, From: {1}"),
					FText::AsNumber(StackCount),
					FText::FromString(SourceName));
			}
		}

		return FText::Format(LOCTEXT("StackCount", "Stacks: {0}"), FText::AsNumber(StackCount));
	}

	return FText::GetEmpty();
}

FName FGASEffectNode::GetLevelStr() const
{
	if (!World || ModSpec)
	{
		return NAME_None;
	}

	return *LexToSanitizedString(GameplayEffect.Spec.GetLevel());
}

FText FGASEffectNode::GetPredictionText() const
{
	if (!World || ModSpec)
	{
		return FText::GetEmpty();
	}

	if (GameplayEffect.PredictionKey.IsValidKey())
	{
		if (GameplayEffect.PredictionKey.WasLocallyGenerated())
		{
			return LOCTEXT("PredictedWaiting", "Predicted and Waiting");
		}
		return LOCTEXT("PredictedCaught", "Predicted and Caught Up");
	}

	return FText::GetEmpty();
}

FName FGASEffectNode::GetGrantedTagsName() const
{
	if (!World || ModSpec)
	{
		return NAME_None;
	}

	FGameplayTagContainer GrantedTags;
	GameplayEffect.Spec.GetAllGrantedTags(GrantedTags);

	if (GrantedTags.IsEmpty())
	{
		return NAME_None;
	}

	return *GrantedTags.ToStringSimple();
}

void FGASEffectNode::PopulateModifiers()
{
	if (!GameplayEffect.Spec.Def)
	{
		return;
	}

	for (int32 ModIdx = 0; ModIdx < GameplayEffect.Spec.Modifiers.Num(); ++ModIdx)
	{
		if (ModIdx < GameplayEffect.Spec.Def->Modifiers.Num())
		{
			AddChildNode(FGASEffectNode::CreateModifier(
				&GameplayEffect.Spec.Modifiers[ModIdx],
				&GameplayEffect.Spec.Def->Modifiers[ModIdx]));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// SGASEffectTreeItem

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGASEffectTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	NodeInfo = InArgs._NodeInfo;
	SetPadding(0);

	check(NodeInfo.IsValid());

	// Cache data
	CachedName = NodeInfo->GetName();
	CachedDurationText = NodeInfo->GetDurationText();
	CachedDurationProgress = NodeInfo->GetDurationProgress();
	CachedStackText = NodeInfo->GetStackText();
	CachedLevelStr = NodeInfo->GetLevelStr();
	CachedPredictionText = NodeInfo->GetPredictionText();
	CachedGrantedTags = NodeInfo->GetGrantedTagsName();
	bIsModifier = NodeInfo->IsModifierNode();

	SMultiColumnTableRow<TSharedRef<FGASEffectNodeBase>>::Construct(
		SMultiColumnTableRow<TSharedRef<FGASEffectNodeBase>>::FArguments().Padding(0),
		InOwnerTableView);
}

TSharedRef<SWidget> SGASEffectTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == GASEffectColumns::Name)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this)).IndentAmount(16).ShouldDrawWires(true)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2.0f, 0.0f).VAlign(VAlign_Center)
			[
				SNew(SBox).HAlign(HAlign_Left).VAlign(VAlign_Center).Padding(FMargin(2.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::FromName(CachedName))
					.ToolTipText(FText::FromName(CachedName))
					.Justification(ETextJustify::Left)
				]
			];
	}
	else if (ColumnName == GASEffectColumns::Duration)
	{
		if (bIsModifier || CachedDurationProgress < 0.0f)
		{
			// For modifiers or infinite duration, just show text
			return SNew(SBox)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(FMargin(2.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(CachedDurationText)
					.ToolTipText(CachedDurationText)
					.Justification(ETextJustify::Left)
				];
		}
		else
		{
			// Show progress bar with dynamic smooth color gradient
			TSharedRef<FGASEffectNodeBase> Node = NodeInfo.ToSharedRef();

			auto GetProgressColor = [Node]() -> FLinearColor
			{
				float Progress = Node->GetDurationProgress();
				if (Progress < 0.0f)
				{
					return FLinearColor::White;
				}

				// Green (>50%) -> Yellow (20-50%) -> Red (<20%)
				if (Progress > 0.5f)
				{
					return FLinearColor::Green;
				}
				else if (Progress > 0.2f)
				{
					return FLinearColor::Yellow;
				}
				else
				{
					return FLinearColor::Red;
				}
			};

			auto GetDurationText = [Node]() -> FText
			{
				return Node->GetDurationText();
			};

			auto GetProgress = [Node]() -> TOptional<float>
			{
				float Progress = Node->GetDurationProgress();
				return Progress >= 0.0f ? TOptional<float>(Progress) : TOptional<float>();
			};

			return SNew(SVerticalBox)
				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SBorder)
					.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FSlateColor(FLinearColor::Black))
					.Padding(0.0f)
					[
						SNew(SProgressBar)
						.Percent_Lambda(GetProgress)
						.FillColorAndOpacity_Lambda([GetProgressColor]() { return FSlateColor(GetProgressColor()); })
						.BackgroundImage(nullptr)
						.FillImage(FAppStyle::GetBrush("WhiteBrush"))
						.BorderPadding(FVector2D::ZeroVector)
					]
				]
				+ SVerticalBox::Slot().AutoHeight().Padding(0, 2, 0, 0)
				[
					SNew(STextBlock)
					.Text_Lambda(GetDurationText)
					.ToolTipText_Lambda(GetDurationText)
					.Justification(ETextJustify::Left)
					.Font(FAppStyle::GetFontStyle("SmallFont"))
				];
		}
	}
	else if (ColumnName == GASEffectColumns::Stack)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(CachedStackText)
				.Justification(ETextJustify::Left)
			];
	}
	else if (ColumnName == GASEffectColumns::Level)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(CachedLevelStr))
				.Justification(ETextJustify::Left)
			];
	}
	else if (ColumnName == GASEffectColumns::Prediction)
	{
		FLinearColor BorderColor = FLinearColor::Transparent;
		if (!CachedPredictionText.IsEmpty())
		{
			// Determine border color based on prediction state
			FString PredictionStr = CachedPredictionText.ToString();
			if (PredictionStr.Contains(TEXT("Waiting")))
			{
				BorderColor = FLinearColor::Blue;
			}
			else if (PredictionStr.Contains(TEXT("Caught Up")))
			{
				BorderColor = FLinearColor::Green;
			}
		}
		return SNew(SBorder)
			.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
			.BorderBackgroundColor(FSlateColor(BorderColor))
			.Padding(FMargin(4.0f, 2.0f))
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(CachedPredictionText)
				.Justification(ETextJustify::Left)
			];
	}
	else if (ColumnName == GASEffectColumns::GrantedTags)
	{
		return SNew(SBox)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(FText::FromName(CachedGrantedTags))
				.ToolTipText(FText::FromName(CachedGrantedTags))
				.Justification(ETextJustify::Left)
			];
	}

	return SNullWidget::NullWidget;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

#undef LOCTEXT_NAMESPACE
