// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssertTags,const FGameplayTagContainer& /*AssertTags*/);

USTRUCT(BlueprintType)
struct FInputTagMapEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayAbilitySpecHandle Handle;

	UPROPERTY()
	FGameplayTag InputTag;
};


/**
 * 
 */
UCLASS()
class AC__API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()

public:
	void AbilityActorInfoSet();
	FEffectAssertTags EffectAssertTags;


	UPROPERTY(ReplicatedUsing = OnRep_InputTagMap)
	TArray<FInputTagMapEntry> InputTagMapArray;
	
	UFUNCTION()
	void OnRep_InputTagMap();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	TMap<FGameplayAbilitySpecHandle, FGameplayTag> InputTagMap;

	void AbilityInputTagHeld(const FGameplayTag& InputTag);
	void AbilityInputTagReleased(const FGameplayTag& InputTag);


	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);


protected:

	UFUNCTION(Client, Unreliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle);
	
};
