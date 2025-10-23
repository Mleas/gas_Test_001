// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WidgetController/OverlayWidgetController.h"
#include <AbilitySystem/AuraAttributeSet.h>

#include "AbilitySystem/AuraAbilitySystemComponent.h"




void UOverlayWidgetController::BroadcastInitialValues()
{
	const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);

    OnHealthChanged.Broadcast(AuraAttributeSet->GetHealth());
    OnMaxHealthChanged.Broadcast(AuraAttributeSet->GetMaxHealth());
    OnManaChanged.Broadcast(AuraAttributeSet->GetMana());
    OnMaxManaChanged.Broadcast(AuraAttributeSet->GetMaxMana());

}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
    const UAuraAttributeSet* AuraAttributeSet = CastChecked<UAuraAttributeSet>(AttributeSet);
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
    {
            OnHealthChanged.Broadcast(Data.NewValue);
    });
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxHealthAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
    {
            OnMaxHealthChanged.Broadcast(Data.NewValue);
    });
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetManaAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
    {
            OnManaChanged.Broadcast(Data.NewValue);
    });
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AuraAttributeSet->GetMaxManaAttribute()).AddLambda(
        [this](const FOnAttributeChangeData& Data)
    {
            OnMaxManaChanged.Broadcast(Data.NewValue);
    });

    Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->EffectAssertTags.AddLambda(
        [this](const FGameplayTagContainer& AssertTags)
        {
            for (const FGameplayTag& Tag : AssertTags)
            {
                FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag("Message");
                if (Tag.MatchesTag(MessageTag))
                {
                    FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable,Tag);

                    MessageWidgetRowDelegate.Broadcast(*Row);
                }
                                                 
            }
        }
    );

}


