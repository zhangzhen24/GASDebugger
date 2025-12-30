// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTask.h"
#include "Widgets/Views/STableViewBase.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Views/STreeView.h"

class UAbilitySystemComponent;
class STableViewBase;

/** Node type in the ability tree */
enum class EGASAbilityNodeType : uint8
{
	Ability,    // Main ability node
	Task,       // Child task node
};

/** Ability state for display */
enum class EGASAbilityState : uint8
{
	Ready,          // Can be activated
	Active,         // Currently active
	Cooldown,       // On cooldown
	InputBlocked,   // Input is blocked
	TagBlocked,     // Tags are blocked
	CantActivate,   // Cannot activate (other reason)
};

/** Column names for the ability tree */
namespace GASAbilityColumns
{
	static const FName Name(TEXT("Name"));
	static const FName State(TEXT("State"));
	static const FName IsActive(TEXT("IsActive"));
	static const FName Triggers(TEXT("Triggers"));
}

/** Base class for ability tree nodes */
class FGASAbilityNodeBase : public TSharedFromThis<FGASAbilityNodeBase>
{
public:
	virtual ~FGASAbilityNodeBase() {}

	/** Get the display name */
	virtual FName GetName() const = 0;

	/** Get the node type */
	virtual EGASAbilityNodeType GetNodeType() const = 0;

	/** Get the current state */
	virtual EGASAbilityState GetState() const = 0;

	/** Get state as display text */
	virtual FText GetStateText() const = 0;

	/** Get state color */
	virtual FLinearColor GetStateColor() const = 0;

	/** Is ability active */
	virtual bool IsActive() const = 0;

	/** Get ability triggers as string */
	virtual FString GetAbilityTriggers() const = 0;

	/** Get asset path for navigation */
	virtual FString GetAssetPath() const = 0;

	/** Check if has valid asset to navigate to */
	virtual bool HasValidAsset() const = 0;

	/** Add a child node */
	void AddChildNode(TSharedRef<FGASAbilityNodeBase> InChild);

	/** Get children */
	const TArray<TSharedRef<FGASAbilityNodeBase>>& GetChildren() const { return Children; }

protected:
	FGASAbilityNodeBase() {}

	TArray<TSharedRef<FGASAbilityNodeBase>> Children;
};

/** Concrete ability node implementation */
class FGASAbilityNode : public FGASAbilityNodeBase
{
public:
	virtual ~FGASAbilityNode() {}

	/** Create an ability node */
	static TSharedRef<FGASAbilityNode> Create(
		TWeakObjectPtr<UAbilitySystemComponent> InASC,
		const FGameplayAbilitySpec& InSpec);

	/** Create a task node */
	static TSharedRef<FGASAbilityNode> CreateTask(
		TWeakObjectPtr<UAbilitySystemComponent> InASC,
		const FGameplayAbilitySpec& InSpec,
		TWeakObjectPtr<UGameplayTask> InTask);

	// FGASAbilityNodeBase interface
	virtual FName GetName() const override;
	virtual EGASAbilityNodeType GetNodeType() const override { return NodeType; }
	virtual EGASAbilityState GetState() const override;
	virtual FText GetStateText() const override;
	virtual FLinearColor GetStateColor() const override;
	virtual bool IsActive() const override;
	virtual FString GetAbilityTriggers() const override;
	virtual FString GetAssetPath() const override;
	virtual bool HasValidAsset() const override;

	/** Update cached state data (call this when refreshing) */
	void UpdateCache();

	// Getter methods for detailed information display

	/** Get ability level */
	int32 GetLevel() const;

	/** Get cooldown information (remaining and total duration) */
	void GetCooldownInfo(float& OutRemaining, float& OutDuration) const;

	/** Get cooldown tags */
	FGameplayTagContainer GetCooldownTags() const;

	/** Get instancing policy as text */
	FText GetInstancingPolicyText() const;

	/** Get net execution policy as text */
	FText GetNetExecutionPolicyText() const;

	/** Get replication policy as text */
	FText GetReplicationPolicyText() const;

	/** Get ability tags */
	FGameplayTagContainer GetAbilityTags() const;

	/** Get activation owned tags */
	FGameplayTagContainer GetActivationOwnedTags() const;

	/** Get activation required tags */
	FGameplayTagContainer GetActivationRequiredTags() const;

	/** Get activation blocked tags */
	FGameplayTagContainer GetActivationBlockedTags() const;

	/** Get source required tags */
	FGameplayTagContainer GetSourceRequiredTags() const;

	/** Get source blocked tags */
	FGameplayTagContainer GetSourceBlockedTags() const;

	/** Get target required tags */
	FGameplayTagContainer GetTargetRequiredTags() const;

	/** Get target blocked tags */
	FGameplayTagContainer GetTargetBlockedTags() const;

	/** Get cancel abilities with tags */
	FGameplayTagContainer GetCancelAbilitiesWithTags() const;

	/** Get cost gameplay effect class */
	const UGameplayEffect* GetCostGameplayEffectClass() const;

	/** Get input ID */
	int32 GetInputID() const;

	/** Check if ability activates on granted */
	bool ActivatesOnGranted() const;

	/** Check if ability activates on input */
	bool ActivatesOnInput() const;

	/** Check if can activate and get failure tags */
	bool CanActivate(FGameplayTagContainer& OutFailureTags) const;

	/** Get source object name */
	FString GetSourceObjectName() const;

	/** Get active instance count */
	int32 GetActiveInstanceCount() const;

	/** Get ability spec handle for identification */
	FGameplayAbilitySpecHandle GetAbilitySpecHandle() const { return AbilitySpec.Handle; }

	/** Get the UGameplayAbility pointer (CDO or instance based on instancing policy) */
	UGameplayAbility* GetAbility() const;

private:
	explicit FGASAbilityNode(
		TWeakObjectPtr<UAbilitySystemComponent> InASC,
		const FGameplayAbilitySpec& InSpec);

	explicit FGASAbilityNode(
		TWeakObjectPtr<UAbilitySystemComponent> InASC,
		const FGameplayAbilitySpec& InSpec,
		TWeakObjectPtr<UGameplayTask> InTask);

	/** Populate ActiveTasks as children */
	void PopulateChildren();

private:
	TWeakObjectPtr<UAbilitySystemComponent> ASC;
	FGameplayAbilitySpec AbilitySpec;
	TWeakObjectPtr<UGameplayTask> GameplayTask;
	EGASAbilityNodeType NodeType = EGASAbilityNodeType::Ability;

	// Cached state for performance
	mutable EGASAbilityState CachedState = EGASAbilityState::Ready;
	mutable bool bStateCached = false;
	mutable FText CachedStateText;
	mutable float CachedCooldownTime = 0.0f;
	mutable int32 CachedActiveCount = 0;
};

/** Tree row widget for abilities */
class SGASAbilityTreeItem : public SMultiColumnTableRow<TSharedRef<FGASAbilityNodeBase>>
{
public:
	SLATE_BEGIN_ARGS(SGASAbilityTreeItem)
		: _NodeInfo()
	{}
		SLATE_ARGUMENT(TSharedPtr<FGASAbilityNodeBase>, NodeInfo)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

protected:
	/** Handle hyperlink click to open asset */
	void HandleHyperlinkNavigate();

	/** Get display text */
	FText GetNameText() const;

	/** Get state color */
	FSlateColor GetStateColor() const;

	/** Get state color as linear color */
	FLinearColor GetStateLinearColor() const;

	/** Get state icon text */
	FText GetStateIconText() const;

private:
	TSharedPtr<FGASAbilityNodeBase> NodeInfo;

	// Cached data
	FName CachedName;
	FText CachedStateText;
	bool bCachedIsActive = false;
	EGASAbilityNodeType CachedNodeType = EGASAbilityNodeType::Ability;
	EGASAbilityState CachedState = EGASAbilityState::Ready;
	FString CachedTriggers;
	FString CachedAssetPath;
	FLinearColor CachedStateColor = FLinearColor::White;
};
