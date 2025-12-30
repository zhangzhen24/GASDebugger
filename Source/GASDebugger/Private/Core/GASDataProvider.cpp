// Copyright Epic Games, Inc. All Rights Reserved.

#include "Core/GASDataProvider.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"
#include "AttributeSet.h"

TArray<FGASAbilityInfo> FGASDataProvider::GetGrantedAbilities(UAbilitySystemComponent* ASC)
{
	TArray<FGASAbilityInfo> Result;

	if (!ASC)
	{
		return Result;
	}

	const TArray<FGameplayAbilitySpec>& ActivatableAbilities = ASC->GetActivatableAbilities();

	for (const FGameplayAbilitySpec& Spec : ActivatableAbilities)
	{
		FGASAbilityInfo Info;
		Info.Handle = Spec.Handle;
		Info.AbilityClass = Spec.Ability ? Spec.Ability->GetClass() : nullptr;
		Info.Level = Spec.Level;
		Info.bIsActive = Spec.IsActive();
		Info.InputID = Spec.InputID;

		// Get cooldown info
		if (Spec.Ability)
		{
			const UGameplayAbility* AbilityCDO = Spec.Ability->GetClass()->GetDefaultObject<UGameplayAbility>();
			if (AbilityCDO)
			{
				float TimeRemaining = 0.0f;
				float CooldownDuration = 0.0f;
				AbilityCDO->GetCooldownTimeRemainingAndDuration(Spec.Handle, ASC->AbilityActorInfo.Get(), TimeRemaining, CooldownDuration);
				Info.CooldownRemaining = TimeRemaining;
				Info.CooldownDuration = CooldownDuration;
			}
		}

		Result.Add(Info);
	}

	return Result;
}

TArray<FGASEffectInfo> FGASDataProvider::GetActiveEffects(UAbilitySystemComponent* ASC)
{
	TArray<FGASEffectInfo> Result;

	if (!ASC)
	{
		return Result;
	}

	TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(FGameplayEffectQuery());

	for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
	{
		const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveEffect)
		{
			continue;
		}

		FGASEffectInfo Info;
		Info.Handle = Handle;
		Info.EffectClass = ActiveEffect->Spec.Def ? ActiveEffect->Spec.Def->GetClass() : nullptr;
		Info.StackCount = ActiveEffect->Spec.GetStackCount();
		Info.Level = ActiveEffect->Spec.GetLevel();

		// Duration info
		float Duration = ActiveEffect->GetDuration();
		if (Duration > 0.0f)
		{
			Info.Duration = Duration;
			Info.TimeRemaining = ActiveEffect->GetTimeRemaining(ASC->GetWorld()->GetTimeSeconds());
		}
		else
		{
			Info.Duration = -1.0f; // Infinite
			Info.TimeRemaining = -1.0f;
		}

		Result.Add(Info);
	}

	return Result;
}

FGameplayTagContainer FGASDataProvider::GetOwnedTags(UAbilitySystemComponent* ASC)
{
	FGameplayTagContainer Result;

	if (ASC)
	{
		ASC->GetOwnedGameplayTags(Result);
	}

	return Result;
}

TArray<FGASAttributeInfo> FGASDataProvider::GetAttributes(UAbilitySystemComponent* ASC)
{
	TArray<FGASAttributeInfo> Result;

	if (!ASC)
	{
		return Result;
	}

	// Get all spawned attributes from ASC
	const TArray<UAttributeSet*>& AttributeSets = ASC->GetSpawnedAttributes();

	for (UAttributeSet* AttrSet : AttributeSets)
	{
		if (!AttrSet)
		{
			continue;
		}

		FName SetName = AttrSet->GetClass()->GetFName();

		// Iterate through all properties to find FGameplayAttributeData
		for (TFieldIterator<FProperty> PropIt(AttrSet->GetClass()); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			if (!Property)
			{
				continue;
			}

			// Check if this is a gameplay attribute
			FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (StructProperty && StructProperty->Struct == FGameplayAttributeData::StaticStruct())
			{
				FGameplayAttribute Attribute(Property);
				if (Attribute.IsValid())
				{
					FGASAttributeInfo Info;
					Info.Attribute = Attribute;
					Info.AttributeSetName = SetName;
					Info.BaseValue = ASC->GetNumericAttributeBase(Attribute);
					Info.CurrentValue = ASC->GetNumericAttribute(Attribute);
					Result.Add(Info);
				}
			}
		}
	}

	return Result;
}

TArray<FGASModifierInfo> FGASDataProvider::GetAttributeModifiers(UAbilitySystemComponent* ASC, const FGameplayAttribute& Attribute)
{
	TArray<FGASModifierInfo> Result;

	if (!ASC || !Attribute.IsValid())
	{
		return Result;
	}

	// Get all active effects and check their modifiers
	TArray<FActiveGameplayEffectHandle> ActiveHandles = ASC->GetActiveEffects(FGameplayEffectQuery());

	for (const FActiveGameplayEffectHandle& Handle : ActiveHandles)
	{
		const FActiveGameplayEffect* ActiveEffect = ASC->GetActiveGameplayEffect(Handle);
		if (!ActiveEffect || !ActiveEffect->Spec.Def)
		{
			continue;
		}

		// Check each modifier in the effect
		for (int32 ModIdx = 0; ModIdx < ActiveEffect->Spec.Def->Modifiers.Num(); ++ModIdx)
		{
			const FGameplayModifierInfo& ModInfo = ActiveEffect->Spec.Def->Modifiers[ModIdx];
			if (ModInfo.Attribute == Attribute)
			{
				FGASModifierInfo Info;
				Info.Operation = ModInfo.ModifierOp;
				Info.SourceEffectClass = ActiveEffect->Spec.Def->GetClass();
				Info.StackCount = ActiveEffect->Spec.GetStackCount();

				// Get evaluated magnitude from the modifier info's magnitude calculation
				float Magnitude = 0.0f;
				ModInfo.ModifierMagnitude.AttemptCalculateMagnitude(ActiveEffect->Spec, Magnitude);
				Info.Magnitude = Magnitude;

				Result.Add(Info);
			}
		}
	}

	return Result;
}
