// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Net/UnrealNetwork.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UAuraAbilitySystemComponent::ClientEffectApplied);
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	
}

void UAuraAbilitySystemComponent::OnRep_InputTagMap()
{
	InputTagMap.Empty();
	for (const FInputTagMapEntry& Entry : InputTagMapArray)
	{
		InputTagMap.Add(Entry.Handle, Entry.InputTag);
	}
}

void UAuraAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAuraAbilitySystemComponent, InputTagMapArray);
}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	for (FGameplayAbilitySpec& AbilitySpec :GetActivatableAbilities())
	{
		
		if (InputTagMap.Contains(AbilitySpec.Handle) && InputTagMap[AbilitySpec.Handle] == InputTag)
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
				TryActivateAbility(AbilitySpec.Handle);
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;;
	
	for (FGameplayAbilitySpec& AbilitySpec :GetActivatableAbilities())
	{
		if (InputTagMap.Contains(AbilitySpec.Handle) && InputTagMap[AbilitySpec.Handle] == InputTag)
			AbilitySpecInputReleased(AbilitySpec);
		
	}
}

void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);
			
			InputTagMap.Add(Handle, AuraAbility->StartInputTag);;
			InputTagMapArray.Add({Handle, AuraAbility->StartInputTag});
		}
		
	}
}

void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssertTags.Broadcast(TagContainer);
	
}

