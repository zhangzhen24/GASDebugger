// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GASDebuggerTypes.h"

class UAbilitySystemComponent;

/**
 * Provides access to GAS data from an AbilitySystemComponent
 * Encapsulates all data queries to avoid direct ASC access in panels
 */
class FGASDataProvider
{
public:
	/**
	 * Get all granted abilities from the ASC
	 * @param ASC The ability system component to query
	 * @return Array of ability information
	 */
	static TArray<FGASAbilityInfo> GetGrantedAbilities(UAbilitySystemComponent* ASC);

	/**
	 * Get all active gameplay effects from the ASC
	 * @param ASC The ability system component to query
	 * @return Array of effect information
	 */
	static TArray<FGASEffectInfo> GetActiveEffects(UAbilitySystemComponent* ASC);

	/**
	 * Get all owned gameplay tags from the ASC
	 * @param ASC The ability system component to query
	 * @return Tag container with all owned tags
	 */
	static FGameplayTagContainer GetOwnedTags(UAbilitySystemComponent* ASC);

	/**
	 * Get all attributes from the ASC
	 * @param ASC The ability system component to query
	 * @return Array of attribute information grouped by AttributeSet
	 */
	static TArray<FGASAttributeInfo> GetAttributes(UAbilitySystemComponent* ASC);

	/**
	 * Get modifiers affecting a specific attribute
	 * @param ASC The ability system component to query
	 * @param Attribute The attribute to get modifiers for
	 * @return Array of modifier information
	 */
	static TArray<FGASModifierInfo> GetAttributeModifiers(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute);
};
