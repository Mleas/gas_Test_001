// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AuraAbilityTypes.h"
#include "Engine/OverlapResult.h"
#include "Game/AuraGameModeBase.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "UI/HUD/AuraHUD.h"

UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject,0))
	{
		if (AAuraHUD* AuraHud = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetParams = FWidgetControllerParams(PC,PS,ASC,AS);
			return AuraHud->GetOverlayWidgetController(WidgetParams);
		}
	
	}
		
	return nullptr;
}

UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject,0))
	{
		if (AAuraHUD* AuraHud = Cast<AAuraHUD>(PC->GetHUD()))
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();
			const FWidgetControllerParams WidgetParams = FWidgetControllerParams(PC,PS,ASC,AS);
			return AuraHud->GetAttributeMenuWidgetController(WidgetParams);
		}
		
		
	}
	return nullptr;
}

void UAuraAbilitySystemLibrary::InitDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level,UAbilitySystemComponent* ASC)
{
	AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGM == nullptr) return;

	AActor* AvtarActor = ASC->GetAvatarActor();
	
	UCharacterClassInfo* CharacterClassInfo = AuraGM->CharacterClassInfo;	
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetCharacterClassInfo(CharacterClass);

	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvtarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes,Level,PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvtarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes,Level,SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	FGameplayEffectContextHandle italAttributesContextHandle = ASC->MakeEffectContext();
	italAttributesContextHandle.AddSourceObject(AvtarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes,Level,italAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGM == nullptr) return;

	UCharacterClassInfo* CharacterClassInfo = AuraGM->CharacterClassInfo;
	if (CharacterClassInfo == nullptr) return;
	for (auto AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	for (auto AbilityClass : CharacterClassInfo->GetCharacterClassInfo(CharacterClass).StartupAbilities)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(ASC->GetAvatarActor()))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, CombatInterface->GetPlayerLevel());
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGM == nullptr) return nullptr;

	return AuraGM->CharacterClassInfo;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
		return AuraEffectContext->IsBlockedhit();
	return false;

}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* AuraEffectContext = static_cast<const FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
		return AuraEffectContext->IsCriticalHit();
	return false;
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsBlockedHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
		AuraEffectContext->SetBlockedhit(bInIsBlockedHit);
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	if (FAuraGameplayEffectContext* AuraEffectContext = static_cast<FAuraGameplayEffectContext*>(EffectContextHandle.Get()))
		AuraEffectContext->SetCriticalHit(bInIsCriticalHit);
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}
