// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AuraGameplayTags.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UAuraAbilitySystemComponent::ClientEffectApplied);
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	


}

void UAuraAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;;

	for (FGameplayAbilitySpec& AbilitySpec :GetActivatableAbilities())
	{
		if (InputTagMap.Contains(AbilitySpec.Handle) && InputTagMap[AbilitySpec.Handle] == InputTag)
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
}

void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;;

	for (FGameplayAbilitySpec& AbilitySpec :GetActivatableAbilities())
	{
		if (InputTagMap.Contains(AbilitySpec.Handle) && InputTagMap[AbilitySpec.Handle] == InputTag)
		{
			AbilitySpecInputReleased(AbilitySpec);
		}
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
void UAuraAbilitySystemComponent::PrintGrantedAbilities()
{
	if (GetActivatableAbilities().Num() == 0)
		return;
		
	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		if (Spec.Ability)
		{
			FString AbilityName = Spec.Ability->GetName();
			UE_LOG(LogTemp, Error, TEXT("Granted Ability: %s"), *AbilityName);
			GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Green,TEXT("Granted Ability"));
		}
	}
}
