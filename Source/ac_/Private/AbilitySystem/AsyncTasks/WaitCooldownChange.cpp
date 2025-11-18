// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"

/**
 * 静态工厂方法，用于创建并初始化一个等待冷却变化的异步任务对象。
 * 
 * @param AbilitySystemComponent 指向能力系统的组件，用于监听冷却标签的变化。
 * @param InCooldownTag 要监听的冷却标签。
 * @return 返回创建的 UWaitCooldownChange 对象，如果参数无效则返回 nullptr。
 */
UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;
	
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// 注册监听冷却标签的新增或移除事件，用于检测冷却结束
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UWaitCooldownChange::CooldownTagChanged);

	// 监听新添加的游戏效果，用于检测冷却开始
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

/**
 * 结束当前任务，清理注册的事件监听器，并标记对象待销毁。
 */
void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

/**
 * 当冷却标签发生变化时的回调函数。
 * 如果标签计数变为0，表示冷却结束，广播 CooldownEnd 事件。
 * 
 * @param InCooldownTag 发生变化的冷却标签。
 * @param NewCount 标签当前的计数。
 */
void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

/**
 * 当有新的游戏效果被添加时的回调函数。
 * 检查该效果是否与当前监听的冷却标签相关，如果相关则广播 CooldownStart 事件。
 * 
 * @param TargetASC 目标能力系统组件。
 * @param SpecApplied 被应用的游戏效果规格。
 * @param ActiveEffectHandle 活跃的游戏效果句柄。
 */
void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		// 查询与冷却标签匹配的活跃效果，获取剩余时间
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// 找到最长的剩余时间并广播
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}
			
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
