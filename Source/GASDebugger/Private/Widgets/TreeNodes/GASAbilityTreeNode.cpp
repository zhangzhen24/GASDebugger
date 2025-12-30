// Copyright Qiu, Inc. All Rights Reserved.

#include "Widgets/TreeNodes/GASAbilityTreeNode.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "SlateOptMacros.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor/EditorEngine.h"
#include "Subsystems/AssetEditorSubsystem.h"
extern UNREALED_API UEditorEngine* GEditor;
#endif

#define LOCTEXT_NAMESPACE "GASAbilityTreeNode"

//////////////////////////////////////////////////////////////////////////
// FGASAbilityNodeBase

void FGASAbilityNodeBase::AddChildNode(TSharedRef<FGASAbilityNodeBase> InChild)
{
	Children.Add(MoveTemp(InChild));
}

//////////////////////////////////////////////////////////////////////////
// FGASAbilityNode

TSharedRef<FGASAbilityNode> FGASAbilityNode::Create(
	TWeakObjectPtr<UAbilitySystemComponent> InASC,
	const FGameplayAbilitySpec& InSpec)
{
	return MakeShareable(new FGASAbilityNode(InASC, InSpec));
}

TSharedRef<FGASAbilityNode> FGASAbilityNode::CreateTask(
	TWeakObjectPtr<UAbilitySystemComponent> InASC,
	const FGameplayAbilitySpec& InSpec,
	TWeakObjectPtr<UGameplayTask> InTask)
{
	return MakeShareable(new FGASAbilityNode(InASC, InSpec, InTask));
}

FGASAbilityNode::FGASAbilityNode(
	TWeakObjectPtr<UAbilitySystemComponent> InASC,
	const FGameplayAbilitySpec& InSpec)
	: ASC(InASC)
	, AbilitySpec(InSpec)
	, NodeType(EGASAbilityNodeType::Ability)
{
	UpdateCache();
	PopulateChildren();
}

FGASAbilityNode::FGASAbilityNode(
	TWeakObjectPtr<UAbilitySystemComponent> InASC,
	const FGameplayAbilitySpec& InSpec,
	TWeakObjectPtr<UGameplayTask> InTask)
	: ASC(InASC)
	, AbilitySpec(InSpec)
	, GameplayTask(InTask)
	, NodeType(EGASAbilityNodeType::Task)
{
}

FName FGASAbilityNode::GetName() const
{
	if (!ASC.IsValid())
	{
		return NAME_None;
	}

	switch (NodeType)
	{
	case EGASAbilityNodeType::Ability:
		if (AbilitySpec.Ability)
		{
			FString CleanedName = ASC->CleanupName(GetNameSafe(AbilitySpec.Ability));
			return *CleanedName;
		}
		return NAME_None;
	case EGASAbilityNodeType::Task:
		if (GameplayTask.IsValid())
		{
			FString DebugString = GameplayTask->GetDebugString();
			return *DebugString;
		}
		return NAME_None;
	}

	return NAME_None;
}

EGASAbilityState FGASAbilityNode::GetState() const
{
	// Use cached state if available
	if (bStateCached)
	{
		return CachedState;
	}

	// Fallback to real-time reading if cache not available (shouldn't happen normally)
	if (!ASC.IsValid())
	{
		return EGASAbilityState::CantActivate;
	}

	// Task nodes don't have their own state
	if (NodeType == EGASAbilityNodeType::Task)
	{
		return EGASAbilityState::Active;
	}

	// Check if active
	if (AbilitySpec.IsActive())
	{
		return EGASAbilityState::Active;
	}

	// Check input blocked
	if (ASC->IsAbilityInputBlocked(AbilitySpec.InputID))
	{
		return EGASAbilityState::InputBlocked;
	}

	// Check tag blocked
	if (AbilitySpec.Ability && ASC->AreAbilityTagsBlocked(AbilitySpec.Ability->GetAssetTags()))
	{
		return EGASAbilityState::TagBlocked;
	}

	// Check can activate
	if (AbilitySpec.Ability && ASC->AbilityActorInfo.IsValid())
	{
		FGameplayTagContainer FailureTags;
		if (!AbilitySpec.Ability->CanActivateAbility(
			AbilitySpec.Handle, ASC->AbilityActorInfo.Get(),
			nullptr, nullptr, &FailureTags))
		{
			// Check if on cooldown
			float Cooldown = AbilitySpec.Ability->GetCooldownTimeRemaining(ASC->AbilityActorInfo.Get());
			if (Cooldown > 0.f)
			{
				return EGASAbilityState::Cooldown;
			}
			return EGASAbilityState::CantActivate;
		}
	}

	return EGASAbilityState::Ready;
}

FText FGASAbilityNode::GetStateText() const
{
	if (NodeType == EGASAbilityNodeType::Task)
	{
		return FText::GetEmpty();
	}

	// Use cached state text if available
	if (bStateCached && !CachedStateText.IsEmpty())
	{
		return CachedStateText;
	}

	// Fallback to real-time reading if cache not available
	EGASAbilityState State = GetState();

	switch (State)
	{
	case EGASAbilityState::Active:
		return FText::Format(LOCTEXT("StateActive", "Active ({0})"), FText::AsNumber(AbilitySpec.ActiveCount));
	case EGASAbilityState::InputBlocked:
		return LOCTEXT("StateInputBlocked", "Input Blocked");
	case EGASAbilityState::TagBlocked:
		return LOCTEXT("StateTagBlocked", "Tag Blocked");
	case EGASAbilityState::Cooldown:
		if (ASC.IsValid() && AbilitySpec.Ability && ASC->AbilityActorInfo.IsValid())
		{
			float Cooldown = AbilitySpec.Ability->GetCooldownTimeRemaining(ASC->AbilityActorInfo.Get());
			return FText::Format(LOCTEXT("StateCooldown", "Cooldown ({0}s)"), FText::AsNumber(FMath::CeilToInt(Cooldown)));
		}
		return LOCTEXT("StateCooldownSimple", "Cooldown");
	case EGASAbilityState::CantActivate:
		return LOCTEXT("StateCantActivate", "Blocked");
	case EGASAbilityState::Ready:
	default:
		return LOCTEXT("StateReady", "Ready");
	}
}

FLinearColor FGASAbilityNode::GetStateColor() const
{
	// Use cached state if available
	if (bStateCached)
	{
		EGASAbilityState State = CachedState;
		switch (State)
		{
		case EGASAbilityState::Active:
			return FLinearColor::Green;
		case EGASAbilityState::Ready:
			return FLinearColor::White;
		case EGASAbilityState::Cooldown:
			return FLinearColor::Yellow;
		case EGASAbilityState::InputBlocked:
		case EGASAbilityState::TagBlocked:
		case EGASAbilityState::CantActivate:
		default:
			return FLinearColor::Red;
		}
	}

	// Fallback to real-time reading
	EGASAbilityState State = GetState();

	switch (State)
	{
	case EGASAbilityState::Active:
		return FLinearColor::Green;
	case EGASAbilityState::Ready:
		return FLinearColor::White;
	case EGASAbilityState::Cooldown:
		return FLinearColor::Yellow;
	case EGASAbilityState::InputBlocked:
	case EGASAbilityState::TagBlocked:
	case EGASAbilityState::CantActivate:
	default:
		return FLinearColor::Red;
	}
}

bool FGASAbilityNode::IsActive() const
{
	return AbilitySpec.IsActive();
}

FString FGASAbilityNode::GetAbilityTriggers() const
{
	if (!ASC.IsValid() || !AbilitySpec.Ability || NodeType != EGASAbilityNodeType::Ability)
	{
		return FString();
	}

	// Get AbilityTriggers via reflection (it's protected)
	FArrayProperty* TriggersPtr = FindFProperty<FArrayProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("AbilityTriggers"));
	if (!TriggersPtr)
	{
		return FString();
	}

	const TArray<FAbilityTriggerData>* Triggers =
		TriggersPtr->ContainerPtrToValuePtr<TArray<FAbilityTriggerData>>(AbilitySpec.Ability);
	if (!Triggers || Triggers->Num() == 0)
	{
		return FString();
	}

	FString Result;
	for (int32 i = 0; i < Triggers->Num(); ++i)
	{
		const FAbilityTriggerData& Trigger = (*Triggers)[i];
		Result += FString::Printf(TEXT("Tag: %s, Source: %s"),
			*Trigger.TriggerTag.ToString(),
			*UEnum::GetDisplayValueAsText(Trigger.TriggerSource).ToString());

		if (i < Triggers->Num() - 1)
		{
			Result += TEXT("\n");
		}
	}

	return Result;
}

FString FGASAbilityNode::GetAssetPath() const
{
	if (NodeType != EGASAbilityNodeType::Ability || !AbilitySpec.Ability)
	{
		return FString();
	}

	UGameplayAbility* Ability = AbilitySpec.Ability;

	// For native assets
	if (Ability->IsAsset())
	{
		return Ability->GetPathName();
	}

	// For Blueprint abilities - get the correct asset path via Class
	UClass* AbilityClass = Ability->GetClass();
	if (AbilityClass)
	{
		// Get the class's package path
		if (UPackage* Package = AbilityClass->GetOuterUPackage())
		{
			FString PackageName = Package->GetName();
			FString ClassName = AbilityClass->GetName();

			// Remove the _C suffix for Blueprint generated classes
			if (ClassName.EndsWith(TEXT("_C")))
			{
				ClassName.LeftChopInline(2);
			}

			// Construct the correct asset path format: /Game/Path/Asset.Asset
			return FString::Printf(TEXT("%s.%s"), *PackageName, *ClassName);
		}
	}

	return FString();
}

bool FGASAbilityNode::HasValidAsset() const
{
	return NodeType == EGASAbilityNodeType::Ability && AbilitySpec.Ability != nullptr;
}

void FGASAbilityNode::PopulateChildren()
{
	if (!ASC.IsValid() || !AbilitySpec.IsActive())
	{
		return;
	}

	TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();

	for (UGameplayAbility* Instance : Instances)
	{
		if (!Instance)
		{
			continue;
		}

		// Get ActiveTasks via reflection (it's protected)
		FArrayProperty* ActiveTasksPtr = FindFProperty<FArrayProperty>(
			Instance->GetClass(), TEXT("ActiveTasks"));
		if (!ActiveTasksPtr)
		{
			continue;
		}

		const TArray<UGameplayTask*>* ActiveTasks =
			ActiveTasksPtr->ContainerPtrToValuePtr<TArray<UGameplayTask*>>(Instance);
		if (!ActiveTasks)
		{
			continue;
		}

		for (UGameplayTask* Task : *ActiveTasks)
		{
			if (Task)
			{
				AddChildNode(FGASAbilityNode::CreateTask(ASC, AbilitySpec, Task));
			}
		}
	}
}

void FGASAbilityNode::UpdateCache()
{
	if (!ASC.IsValid())
	{
		bStateCached = false;
		return;
	}

	// Task nodes don't have their own state
	if (NodeType == EGASAbilityNodeType::Task)
	{
		CachedState = EGASAbilityState::Active;
		bStateCached = true;
		CachedStateText = FText::GetEmpty();
		return;
	}

	// Calculate state
	if (AbilitySpec.IsActive())
	{
		CachedState = EGASAbilityState::Active;
		CachedActiveCount = AbilitySpec.ActiveCount;
		CachedStateText = FText::Format(LOCTEXT("StateActive", "Active ({0})"), FText::AsNumber(CachedActiveCount));
	}
	else if (ASC->IsAbilityInputBlocked(AbilitySpec.InputID))
	{
		CachedState = EGASAbilityState::InputBlocked;
		CachedStateText = LOCTEXT("StateInputBlocked", "Input Blocked");
	}
	else if (AbilitySpec.Ability && ASC->AreAbilityTagsBlocked(AbilitySpec.Ability->GetAssetTags()))
	{
		CachedState = EGASAbilityState::TagBlocked;
		CachedStateText = LOCTEXT("StateTagBlocked", "Tag Blocked");
	}
	else if (AbilitySpec.Ability && ASC->AbilityActorInfo.IsValid())
	{
		FGameplayTagContainer FailureTags;
		if (!AbilitySpec.Ability->CanActivateAbility(
			AbilitySpec.Handle, ASC->AbilityActorInfo.Get(),
			nullptr, nullptr, &FailureTags))
		{
			// Check if on cooldown
			CachedCooldownTime = AbilitySpec.Ability->GetCooldownTimeRemaining(ASC->AbilityActorInfo.Get());
			if (CachedCooldownTime > 0.f)
			{
				CachedState = EGASAbilityState::Cooldown;
				CachedStateText = FText::Format(LOCTEXT("StateCooldown", "Cooldown ({0}s)"), FText::AsNumber(FMath::CeilToInt(CachedCooldownTime)));
			}
			else
			{
				CachedState = EGASAbilityState::CantActivate;
				CachedStateText = LOCTEXT("StateCantActivate", "Blocked");
			}
		}
		else
		{
			CachedState = EGASAbilityState::Ready;
			CachedStateText = LOCTEXT("StateReady", "Ready");
		}
	}
	else
	{
		CachedState = EGASAbilityState::Ready;
		CachedStateText = LOCTEXT("StateReady", "Ready");
	}

	bStateCached = true;
}

//////////////////////////////////////////////////////////////////////////
// SGASAbilityTreeItem

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGASAbilityTreeItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	NodeInfo = InArgs._NodeInfo;
	SetPadding(0);

	check(NodeInfo.IsValid());

	// Cache data
	CachedName = NodeInfo->GetName();
	CachedStateText = NodeInfo->GetStateText();
	bCachedIsActive = NodeInfo->IsActive();
	CachedNodeType = NodeInfo->GetNodeType();
	CachedState = NodeInfo->GetState();
	CachedTriggers = NodeInfo->GetAbilityTriggers();
	CachedAssetPath = NodeInfo->GetAssetPath();
	CachedStateColor = NodeInfo->GetStateColor();

	SMultiColumnTableRow<TSharedRef<FGASAbilityNodeBase>>::Construct(
		SMultiColumnTableRow<TSharedRef<FGASAbilityNodeBase>>::FArguments().Padding(0),
		InOwnerTableView);
}

TSharedRef<SWidget> SGASAbilityTreeItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == GASAbilityColumns::Name)
	{
		TSharedPtr<SWidget> NameWidget;

		if (CachedNodeType == EGASAbilityNodeType::Ability && NodeInfo->HasValidAsset())
		{
			// Hyperlink for navigating to asset
			NameWidget = SNew(SHyperlink)
				.Text(this, &SGASAbilityTreeItem::GetNameText)
				.ToolTipText(FText::FromString(CachedAssetPath))
				.OnNavigate(this, &SGASAbilityTreeItem::HandleHyperlinkNavigate);
		}
		else
		{
			// Plain text for tasks
			NameWidget = SNew(STextBlock)
				.Text(this, &SGASAbilityTreeItem::GetNameText)
				.ToolTipText(this, &SGASAbilityTreeItem::GetNameText);
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SExpanderArrow, SharedThis(this))
				.IndentAmount(16)
				.ShouldDrawWires(true)
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2.0f, 0.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SBorder)
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Visibility(EVisibility::SelfHitTestInvisible)
				.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
				.ColorAndOpacity(this, &SGASAbilityTreeItem::GetStateLinearColor)
				[
					NameWidget.ToSharedRef()
				]
			];
	}
	else if (ColumnName == GASAbilityColumns::State)
	{
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(4, 0, 4, 0)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(this, &SGASAbilityTreeItem::GetStateIconText)
				.ColorAndOpacity(FSlateColor(CachedStateColor))
				.Font(FAppStyle::GetFontStyle("Bold"))
			]
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(CachedStateText)
				.ColorAndOpacity(FSlateColor(CachedStateColor))
				.Justification(ETextJustify::Left)
			];
	}
	else if (ColumnName == GASAbilityColumns::IsActive)
	{
		FText ActiveText = bCachedIsActive
			? LOCTEXT("ActiveYes", "Yes")
			: LOCTEXT("ActiveNo", "No");

		return SNew(SBorder)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			.Visibility(EVisibility::SelfHitTestInvisible)
			.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
			.ColorAndOpacity(this, &SGASAbilityTreeItem::GetStateLinearColor)
			[
				SNew(STextBlock)
				.Text(ActiveText)
				.Justification(ETextJustify::Left)
			];
	}
	else if (ColumnName == GASAbilityColumns::Triggers)
	{
		return SNew(SBorder)
			.HAlign(HAlign_Left)
			.VAlign(VAlign_Center)
			.Padding(FMargin(2.0f, 0.0f))
			.Visibility(EVisibility::SelfHitTestInvisible)
			.BorderBackgroundColor(FSlateColor(FLinearColor(1.f, 1.f, 1.f, 0.f)))
			[
				SNew(STextBlock)
				.Text(FText::FromString(CachedTriggers))
				.ToolTipText(FText::FromString(CachedTriggers))
				.Justification(ETextJustify::Left)
			];
	}

	return SNullWidget::NullWidget;
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

void SGASAbilityTreeItem::HandleHyperlinkNavigate()
{
	if (CachedAssetPath.IsEmpty())
	{
		return;
	}

#if WITH_EDITOR
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(FSoftObjectPath(*CachedAssetPath));

	if (AssetData.IsValid())
	{
		UObject* Asset = AssetData.GetAsset();
		if (Asset)
		{
			GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Asset);
		}
	}
#endif
}

FText SGASAbilityTreeItem::GetNameText() const
{
	return FText::FromName(CachedName);
}

FSlateColor SGASAbilityTreeItem::GetStateColor() const
{
	return FSlateColor(CachedStateColor);
}

FLinearColor SGASAbilityTreeItem::GetStateLinearColor() const
{
	return CachedStateColor;
}

FText SGASAbilityTreeItem::GetStateIconText() const
{
	if (CachedNodeType == EGASAbilityNodeType::Task)
	{
		return FText::GetEmpty();
	}

	switch (CachedState)
	{
	case EGASAbilityState::Active:
		return FText::FromString(TEXT("[A]"));  // Active
	case EGASAbilityState::Cooldown:
		return FText::FromString(TEXT("[CD]"));  // Cooldown
	case EGASAbilityState::InputBlocked:
	case EGASAbilityState::TagBlocked:
		return FText::FromString(TEXT("[X]"));  // Blocked
	case EGASAbilityState::CantActivate:
		return FText::FromString(TEXT("[!]"));  // Warning
	case EGASAbilityState::Ready:
	default:
		return FText::FromString(TEXT("[R]"));  // Ready
	}
}

//////////////////////////////////////////////////////////////////////////
// FGASAbilityNode - Detailed Information Getters

int32 FGASAbilityNode::GetLevel() const
{
	return AbilitySpec.Level;
}

void FGASAbilityNode::GetCooldownInfo(float& OutRemaining, float& OutDuration) const
{
	OutRemaining = 0.0f;
	OutDuration = 0.0f;

	if (!AbilitySpec.Ability || !ASC.IsValid() || !ASC->AbilityActorInfo.IsValid())
	{
		return;
	}

	// Get remaining cooldown time
	OutRemaining = AbilitySpec.Ability->GetCooldownTimeRemaining(ASC->AbilityActorInfo.Get());

	// Get total cooldown duration from cooldown gameplay effect
	const UGameplayEffect* CooldownGE = AbilitySpec.Ability->GetCooldownGameplayEffect();
	if (CooldownGE)
	{
		// Try to get duration magnitude
		OutDuration = CooldownGE->DurationMagnitude.GetStaticMagnitudeIfPossible(1.0f, OutDuration);
	}
}

FGameplayTagContainer FGASAbilityNode::GetCooldownTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	const FGameplayTagContainer* CooldownTags = AbilitySpec.Ability->GetCooldownTags();
	if (CooldownTags)
	{
		return *CooldownTags;
	}

	return FGameplayTagContainer();
}

FText FGASAbilityNode::GetInstancingPolicyText() const
{
	if (!AbilitySpec.Ability)
	{
		return LOCTEXT("Unknown", "Unknown");
	}

	switch (AbilitySpec.Ability->GetInstancingPolicy())
	{
	case EGameplayAbilityInstancingPolicy::InstancedPerActor:
		return LOCTEXT("PolicyInstancedPerActor", "InstancedPerActor");
	case EGameplayAbilityInstancingPolicy::InstancedPerExecution:
		return LOCTEXT("PolicyInstancedPerExecution", "InstancedPerExecution");
	default:
		return LOCTEXT("Unknown", "Unknown");
	}
}

FText FGASAbilityNode::GetNetExecutionPolicyText() const
{
	if (!AbilitySpec.Ability)
	{
		return LOCTEXT("Unknown", "Unknown");
	}

	switch (AbilitySpec.Ability->GetNetExecutionPolicy())
	{
	case EGameplayAbilityNetExecutionPolicy::LocalPredicted:
		return LOCTEXT("NetExecLocalPredicted", "LocalPredicted");
	case EGameplayAbilityNetExecutionPolicy::LocalOnly:
		return LOCTEXT("NetExecLocalOnly", "LocalOnly");
	case EGameplayAbilityNetExecutionPolicy::ServerInitiated:
		return LOCTEXT("NetExecServerInitiated", "ServerInitiated");
	case EGameplayAbilityNetExecutionPolicy::ServerOnly:
		return LOCTEXT("NetExecServerOnly", "ServerOnly");
	default:
		return LOCTEXT("Unknown", "Unknown");
	}
}

FText FGASAbilityNode::GetReplicationPolicyText() const
{
	if (!AbilitySpec.Ability)
	{
		return LOCTEXT("Unknown", "Unknown");
	}

	switch (AbilitySpec.Ability->GetReplicationPolicy())
	{
	case EGameplayAbilityReplicationPolicy::ReplicateNo:
		return LOCTEXT("ReplicationNo", "ReplicateNo");
	case EGameplayAbilityReplicationPolicy::ReplicateYes:
		return LOCTEXT("ReplicationYes", "ReplicateYes");
	default:
		return LOCTEXT("Unknown", "Unknown");
	}
}

FGameplayTagContainer FGASAbilityNode::GetAbilityTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Use GetAssetTags() which is the public API
	return AbilitySpec.Ability->GetAssetTags();
}

FGameplayTagContainer FGASAbilityNode::GetActivationOwnedTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("ActivationOwnedTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetActivationRequiredTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("ActivationRequiredTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetActivationBlockedTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("ActivationBlockedTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetSourceRequiredTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("SourceRequiredTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetSourceBlockedTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("SourceBlockedTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetTargetRequiredTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("TargetRequiredTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetTargetBlockedTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("TargetBlockedTags"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

FGameplayTagContainer FGASAbilityNode::GetCancelAbilitiesWithTags() const
{
	if (!AbilitySpec.Ability)
	{
		return FGameplayTagContainer();
	}

	// Access protected member via reflection
	FStructProperty* TagsPtr = FindFProperty<FStructProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("CancelAbilitiesWithTag"));
	if (TagsPtr)
	{
		const FGameplayTagContainer* Tags =
			TagsPtr->ContainerPtrToValuePtr<FGameplayTagContainer>(AbilitySpec.Ability);
		if (Tags)
		{
			return *Tags;
		}
	}

	return FGameplayTagContainer();
}

const UGameplayEffect* FGASAbilityNode::GetCostGameplayEffectClass() const
{
	if (!AbilitySpec.Ability)
	{
		return nullptr;
	}

	return AbilitySpec.Ability->GetCostGameplayEffect();
}

int32 FGASAbilityNode::GetInputID() const
{
	return AbilitySpec.InputID;
}

bool FGASAbilityNode::ActivatesOnGranted() const
{
	if (!AbilitySpec.Ability)
	{
		return false;
	}

	// Access protected member via reflection since bActivateAbilityOnGranted may not exist in UE 5.4
	FBoolProperty* BoolPtr = FindFProperty<FBoolProperty>(
		AbilitySpec.Ability->GetClass(), TEXT("bActivateAbilityOnGranted"));
	if (BoolPtr)
	{
		return BoolPtr->GetPropertyValue_InContainer(AbilitySpec.Ability);
	}

	return false;
}

bool FGASAbilityNode::ActivatesOnInput() const
{
	if (!AbilitySpec.Ability)
	{
		return false;
	}

	// Check if ability uses input binding
	return AbilitySpec.InputID != INDEX_NONE;
}

bool FGASAbilityNode::CanActivate(FGameplayTagContainer& OutFailureTags) const
{
	OutFailureTags.Reset();

	if (!AbilitySpec.Ability || !ASC.IsValid() || !ASC->AbilityActorInfo.IsValid())
	{
		return false;
	}

	return AbilitySpec.Ability->CanActivateAbility(
		AbilitySpec.Handle,
		ASC->AbilityActorInfo.Get(),
		nullptr,
		nullptr,
		&OutFailureTags);
}

FString FGASAbilityNode::GetSourceObjectName() const
{
	if (AbilitySpec.SourceObject.IsValid())
	{
		return AbilitySpec.SourceObject->GetName();
	}

	return TEXT("None");
}

int32 FGASAbilityNode::GetActiveInstanceCount() const
{
	return AbilitySpec.ActiveCount;
}

UGameplayAbility* FGASAbilityNode::GetAbility() const
{
	if (!AbilitySpec.Ability)
	{
		return nullptr;
	}

	// 根据实例化策略返回合适的 Ability 对象
	EGameplayAbilityInstancingPolicy::Type Policy = AbilitySpec.Ability->GetInstancingPolicy();

	if (Policy == EGameplayAbilityInstancingPolicy::InstancedPerActor ||
	    Policy == EGameplayAbilityInstancingPolicy::InstancedPerExecution)
	{
		// 尝试获取已实例化的 Ability
		TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		if (Instances.Num() > 0 && Instances[0])
		{
			// 返回第一个实例
			return Instances[0];
		}
		// 如果没有实例，返回 CDO
		return AbilitySpec.Ability;
	}

	// 其他策略（包括 NonInstanced）直接返回 CDO
	return AbilitySpec.Ability;
}

#undef LOCTEXT_NAMESPACE
