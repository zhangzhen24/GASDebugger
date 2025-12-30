// Copyright Qiu, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "ActiveGameplayEffectHandle.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"

/**
 * Debug categories for GAS debugging
 */
enum class EGASDebugCategory : uint8
{
	Tags,
	Attributes,
	GameplayEffects,
	Ability
};

/**
 * Filter state for ability display
 */
enum EGASAbilityFilterState : uint8
{
	Active = 1,
	Blocked = 1 << 1,
	Inactive = 1 << 2
};

/**
 * Information about a granted gameplay ability
 */
struct FGASAbilityInfo
{
	/** The ability spec handle */
	FGameplayAbilitySpecHandle Handle;

	/** The ability class */
	TSubclassOf<class UGameplayAbility> AbilityClass;

	/** Ability level */
	int32 Level = 0;

	/** Is the ability currently active */
	bool bIsActive = false;

	/** Remaining cooldown time (0 if not on cooldown) */
	float CooldownRemaining = 0.0f;

	/** Total cooldown duration */
	float CooldownDuration = 0.0f;

	/** Input ID if bound */
	int32 InputID = INDEX_NONE;
};

/**
 * Information about an active gameplay effect
 */
struct FGASEffectInfo
{
	/** The active effect handle */
	FActiveGameplayEffectHandle Handle;

	/** The effect class */
	TSubclassOf<class UGameplayEffect> EffectClass;

	/** Effect duration (-1 for infinite) */
	float Duration = 0.0f;

	/** Remaining time */
	float TimeRemaining = 0.0f;

	/** Current stack count */
	int32 StackCount = 0;

	/** Effect level */
	float Level = 0.0f;
};

/**
 * Information about a gameplay attribute
 */
struct FGASAttributeInfo
{
	/** The attribute */
	FGameplayAttribute Attribute;

	/** Base value */
	float BaseValue = 0.0f;

	/** Current value (with modifiers) */
	float CurrentValue = 0.0f;

	/** Name of the owning AttributeSet */
	FName AttributeSetName;
};

/**
 * Information about an attribute modifier
 */
struct FGASModifierInfo
{
	/** Modifier operation type */
	EGameplayModOp::Type Operation = EGameplayModOp::Additive;

	/** Modifier magnitude */
	float Magnitude = 0.0f;

	/** Source effect class */
	TSubclassOf<class UGameplayEffect> SourceEffectClass;

	/** Stack count from source */
	int32 StackCount = 0;
};
