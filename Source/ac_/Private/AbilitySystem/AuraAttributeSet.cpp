// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAttributeSet.h"

#include <Net/UnrealNetwork.h>
#include  "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Interface/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerController.h"


UAuraAttributeSet::UAuraAttributeSet()
{

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	//Primary
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Strength,GetStrengthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Intelligence,GetIntelligenceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Resilience,GetResilienceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Primary_Vigor,GetVigorAttribute);


	//Secondary
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_Armor, UAuraAttributeSet::GetArmorAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ArmorPenetration, UAuraAttributeSet::GetArmorPenetrationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_BlockChance, UAuraAttributeSet::GetBlockChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitChance, UAuraAttributeSet::GetCriticalHitChanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitDamage, UAuraAttributeSet::GetCriticalHitDamageAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_CriticalHitResistance, UAuraAttributeSet::GetCriticalHitResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, UAuraAttributeSet::GetHealthRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, UAuraAttributeSet::GetManaRegenerationAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, UAuraAttributeSet::GetMaxHealthAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, UAuraAttributeSet::GetMaxManaAttribute);

	//Resistances
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Fire, UAuraAttributeSet::GetFireResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Lightning, UAuraAttributeSet::GetLightningResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Arcane, UAuraAttributeSet::GetArcaneResistanceAttribute);
	TagsToAttributes.Add(GameplayTags.Attributes_Resistance_Physical, UAuraAttributeSet::GetPhysicalResistanceAttribute);

	
}

void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Resilience, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Vigor, COND_None, REPNOTIFY_Always);

	
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	

	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Armor, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArmorPenetration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, BlockChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitChance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitDamage, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, CriticalHitResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);

	//Resistances
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, FireResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, LightningResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ArcaneResistance, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, PhysicalResistance, COND_None, REPNOTIFY_Always);

}

void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxHealth());
	if (Attribute == GetManaAttribute())
		NewValue = FMath::Clamp<float>(NewValue, 0.f, GetMaxMana());
}

void UAuraAttributeSet::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	FEffectProperties Props;
	SetEffectProperties(Data, Props);

	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(),0.f,GetMaxHealth()));
		UE_LOG(LogTemp, Warning, TEXT("Health Changed on %s, Health: %f"), *Props.TargetAvatarActor->GetName(), GetHealth());
	}
	
	if (Data.EvaluatedData.Attribute == GetManaAttribute())
		SetMana(FMath::Clamp(GetMana(),0.f,GetMaxMana()));
	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		const float LocalIncomingDamage = GetIncomingDamage();
		SetIncomingDamage(0.f);
		if (LocalIncomingDamage > 0.f)
		{
			float TempDamage = LocalIncomingDamage;
			const float NewHealth = GetHealth() - LocalIncomingDamage;
			//
			SetHealth(FMath::Clamp(NewHealth,0.f,GetMaxHealth()));

			const bool bFatal = NewHealth <= 0.f;
			if (bFatal)
			{
				ICombatInterface* CombatInterface = Cast<ICombatInterface>(Props.TargetAvatarActor);
				if (CombatInterface)
					CombatInterface->Die();
			}
			else
			{
				FGameplayTagContainer Tags;
				Tags.AddTag(FAuraGameplayTags::Get().Effects_HitReact);
				Props.TargetASC->TryActivateAbilitiesByTag(Tags);
			}

			//Show Damage Text
			const bool bCriticalHit = UAuraAbilitySystemLibrary::IsCriticalHit(Props.EffectContextHandle);
			const bool bBlockedHit = UAuraAbilitySystemLibrary::IsBlockedHit(Props.EffectContextHandle);
			ShowFloatingText(Props,LocalIncomingDamage, bBlockedHit, bCriticalHit);
		}
	}
}
void UAuraAttributeSet::ShowFloatingText(const FEffectProperties& Props, float Damage, bool bBlockedHit,
	bool bCriticalHit) const
{
	if (Props.SourceCharacter != Props.TargetCharacter)
	{
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.SourceCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter,bBlockedHit,bCriticalHit);
			return;
		}
		if (AAuraPlayerController* PC = Cast<AAuraPlayerController>(Props.TargetCharacter->Controller))
		{
			PC->ShowDamageNumber(Damage, Props.TargetCharacter,bBlockedHit,bCriticalHit);
		}
	}
}

void UAuraAttributeSet::OnRep_Strength(const FGameplayAttributeData& OldStrength)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}

void UAuraAttributeSet::OnRep_Intelligence(const FGameplayAttributeData& OldIntelligence)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Intelligence, OldIntelligence);
}

void UAuraAttributeSet::OnRep_Resilience(const FGameplayAttributeData& OldResilience)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Resilience, OldResilience);
}

void UAuraAttributeSet::OnRep_Vigor(const FGameplayAttributeData& OldVigor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Vigor, OldVigor);
}

void UAuraAttributeSet::OnRep_Armor(const FGameplayAttributeData& OldArmor)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Armor, OldArmor);
}

void UAuraAttributeSet::OnRep_ArmorPenetration(const FGameplayAttributeData& OldArmorPenetration)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArmorPenetration, OldArmorPenetration);
}

void UAuraAttributeSet::OnRep_BlockChance(const FGameplayAttributeData& OldBlockChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, BlockChance, OldBlockChance);
}

void UAuraAttributeSet::OnRep_CriticalHitChance(const FGameplayAttributeData& OldCriticalHitChance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitChance, OldCriticalHitChance);
}

void UAuraAttributeSet::OnRep_CriticalHitDamage(const FGameplayAttributeData& OldCriticalHitDamage)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitDamage, OldCriticalHitDamage);
}

void UAuraAttributeSet::OnRep_CriticalHitResistance(const FGameplayAttributeData& OldCriticalHitResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, CriticalHitResistance, OldCriticalHitResistance);
}

void UAuraAttributeSet::OnRep_HealthRegeneration(const FGameplayAttributeData& OldHealthRegeneration)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, HealthRegeneration, OldHealthRegeneration);
}

void UAuraAttributeSet::OnRep_ManaRegeneration(const FGameplayAttributeData& OldManaRegeneration)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ManaRegeneration, OldManaRegeneration);
}

void UAuraAttributeSet::OnRep_Health(const FGameplayAttributeData& OldHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

void UAuraAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxHealth, OldMaxHealth);

}

void UAuraAttributeSet::OnRep_Mana(const FGameplayAttributeData& OldMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Mana, OldMana);

}


void UAuraAttributeSet::OnRep_MaxMana(const FGameplayAttributeData& OldMaxMana)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxMana, OldMaxMana);

}

// Resistances
//Fire
void UAuraAttributeSet::OnRep_FireResistance(const FGameplayAttributeData& OldFireResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, FireResistance, OldFireResistance);
}
//Lightning
void UAuraAttributeSet::OnRep_LightningResistance(const FGameplayAttributeData& OldLightningResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, LightningResistance, OldLightningResistance);
}
//Arcane
void UAuraAttributeSet::OnRep_ArcaneResistance(const FGameplayAttributeData& OldArcaneResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, ArcaneResistance, OldArcaneResistance);
}
//Physical
void UAuraAttributeSet::OnRep_PhysicalResistance(const FGameplayAttributeData& OldPhysicalResistance)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, PhysicalResistance, OldPhysicalResistance);
}


void UAuraAttributeSet::SetEffectProperties(const struct FGameplayEffectModCallbackData& Data,
                                            FEffectProperties& Props) const
{
	Props.EffectContextHandle = Data.EffectSpec.GetContext();
	Props.SourceASC = Props.EffectContextHandle.GetInstigatorAbilitySystemComponent();

	if (IsValid(Props.SourceASC) && Props.SourceASC->AbilityActorInfo.IsValid() && Props.SourceASC->AbilityActorInfo.IsValid())
	{
		Props.SourceAvatarActor = Props.SourceASC->AbilityActorInfo->AvatarActor.Get();
		Props.SourceController = Props.SourceASC->AbilityActorInfo->PlayerController.Get();
		if (Props.SourceController == nullptr && Props.SourceAvatarActor != nullptr)
		{
			if (APawn* Pawn = Cast<APawn>(Props.SourceAvatarActor))
			{
				Props.SourceController = Pawn->GetController();
			}
		}
		if (Props.SourceController != nullptr)
		{

			Props.SourceCharacter = Cast<ACharacter>(Props.SourceController->GetPawn());
		}
	}
	if (Data.Target.AbilityActorInfo.IsValid() && Data.Target.AbilityActorInfo->AvatarActor.IsValid())
	{
		Props.TargetAvatarActor = Data.Target.AbilityActorInfo->AvatarActor.Get();
		Props.TargetController = Data.Target.AbilityActorInfo->PlayerController.Get();
		Props.TargetCharacter = Cast<ACharacter>(Props.TargetAvatarActor);
		Props.TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Props.TargetAvatarActor);
	}
}


