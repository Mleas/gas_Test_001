// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AuraAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "ac_/AuraLogChannels.h"
#include "Interface/PlayerInterface.h"
#include "Net/UnrealNetwork.h"
/**
 * 当AbilityActor信息设置完成时调用此函数。
 * 注册了游戏特效应用到自身的委托，并获取游戏玩法标签引用。
 */
void UAuraAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this,&UAuraAbilitySystemComponent::ClientEffectApplied);
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	
}

/**
 * 复制输入标签映射数组到本地的TMap结构中。
 * 在网络复制过程中被调用，确保客户端与服务器端的数据一致性。
 */
void UAuraAbilitySystemComponent::OnRep_InputTagMap()
{
	InputTagMap.Empty();
	for (const FInputTagMapEntry& Entry : InputTagMapArray)
	{
		InputTagMap.Add(Entry.Handle, Entry.InputTag);
	}
}

/**
 * 定义需要在网络间复制的属性列表。
 * 包含InputTagMapArray以支持多播同步。
 *
 * @param OutLifetimeProps 输出参数，用于存储需复制的属性列表
 */
void UAuraAbilitySystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UAuraAbilitySystemComponent, InputTagMapArray);
}

/**
 * 处理能力按键按下的逻辑。
 * 查找对应的能力规格并激活它（如果未处于活动状态）。
 *
 * @param InputTag 输入标签，标识要触发的能力类型
 */
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

/**
 * 处理能力按键释放的逻辑。
 * 查找对应的能力规格并通知其输入已被释放。
 *
 * @param InputTag 输入标签，标识要停止的能力类型
 */
void UAuraAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;;
	
	for (FGameplayAbilitySpec& AbilitySpec :GetActivatableAbilities())
	{
		if (InputTagMap.Contains(AbilitySpec.Handle) && InputTagMap[AbilitySpec.Handle] == InputTag)
			AbilitySpecInputReleased(AbilitySpec);
		
	}
}

/**
 * 遍历所有可用能力并执行指定的委托操作。
 * 使用范围锁保证线程安全访问能力列表。
 *
 * @param Delegate 委托对象，定义每次迭代要执行的操作
 */
void UAuraAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogAura, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

/**
 * 添加角色初始主动能力集合。
 * 给予每个能力一个动态源标签，并将其句柄和输入标签记录下来以便后续查找。
 *
 * @param StartupAbilities 初始能力类数组
 */
void UAuraAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
		
		if (const UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability))
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(AuraAbility->StartInputTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Equipped);
			FGameplayAbilitySpecHandle Handle = GiveAbility(AbilitySpec);
			
			InputTagMap.Add(Handle, AuraAbility->StartInputTag);;
			InputTagMapArray.Add({Handle, AuraAbility->StartInputTag});
		}
		
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast();
}

/**
 * 添加角色被动能力集合。
 * 直接给予并尝试一次性激活这些能力。
 *
 * @param StartupPassiveAbilities 被动能力类数组
 */
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass,1);
			
		GiveAbilityAndActivateOnce(AbilitySpec);
			
	}
}

/**
 * 从能力规格中提取状态标签。
 * 寻找匹配"Abilities.Status"前缀的游戏标签作为状态返回。
 *
 * @param AbilitySpec 能力规格引用
 * @return 返回找到的状态标签或空标签
 */
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	for (FGameplayTag StatusTag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (StatusTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return StatusTag;
		}
	}
	return FGameplayTag();
}

/**
 * 实现客户端效果应用处理函数。
 * 收集特效的所有资产标签并通过广播事件传递给监听者。
 *
 * @param AbilitySystemComponent 应用特效的能力系统组件指针
 * @param EffectSpec 特效规范引用
 * @param ActiveGameplayEffectHandle 激活的特效句柄
 */
void UAuraAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                     const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssertTags.Broadcast(TagContainer);
	
}

/**
 * 从能力规格中提取能力标签。
 * 查找能力资产标签中符合"Abilities"前缀的第一个标签。
 *
 * @param AbilitySpec 能力规格引用
 * @return 返回找到的能力标签或空标签
 */
FGameplayTag UAuraAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		const FGameplayTagContainer& AssetTags = AbilitySpec.Ability->GetAssetTags();
		const FGameplayTag AbilitiesTag = FGameplayTag::RequestGameplayTag(TEXT("Abilities"));

		for (const FGameplayTag& Tag : AssetTags)
		{
			if (Tag.MatchesTag(AbilitiesTag))
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

/**
 * 从能力规格中提取输入标签。
 * 查找动态源标签中符合"Input"前缀的标签。
 *
 * @param AbilitySpec 能力规格引用
 * @return 返回找到的输入标签或空标签
 */
FGameplayTag UAuraAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{/*
	UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec.Ability);
	if (!AuraAbility) return FGameplayTag();
	if (AuraAbility->StartInputTag.IsValid() && AuraAbility->StartInputTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input"))))
	{
		return AuraAbility->StartInputTag;
	}*/
	for (FGameplayTag Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Input"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

/**
 * 服务端实现：升级指定属性。
 * 发送游戏事件并减少玩家属性点数。
 *
 * @param AttributeTag 属性标签，表示要升级的具体属性
 */
void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

/**
 * 请求升级指定属性。
 * 先验证是否有足够的属性点再发起服务端请求。
 *
 * @param AttributeTag 属性标签，表示要升级的具体属性
 */
void UAuraAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

/**
 * 更新角色能力状态
 * 
 * 该函数根据角色当前等级检查并更新所有可用能力的状态。
 * 对于满足等级要求但尚未获得的能力，将添加到角色的能力系统中并标记为可使用状态。
 * 
 * @param Level 角色当前等级，用于判断能力是否满足解锁条件
 */
void UAuraAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	// 获取能力信息配置数据
	UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	
	// 遍历所有能力信息，检查并更新符合条件的能力状态
	for (const FAuraAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		// 跳过无效的能力标签
		if (!Info.AbilityTag.IsValid()) continue;
		// 跳过不满足等级要求的能力
		if (Level < Info.LevelRequirement) continue;
		
		// 检查角色是否已拥有该能力，如果没有则添加新能力
		if (GetSpecFromAbilityTag(Info.AbilityTag) == nullptr)
		{
			// 创建新的能力规格实例
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			// 设置能力状态标签为"可使用"
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FAuraGameplayTags::Get().Abilities_Status_Eligible);
			// 将能力添加到角色能力系统中
			GiveAbility(AbilitySpec);
			// 标记能力规格为脏数据，需要同步更新
			MarkAbilitySpecDirty(AbilitySpec);
			// 向客户端发送能力状态更新通知
			ClientUpdateAbilityStatus(Info.AbilityTag,FAuraGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

/**
 * 根据能力标签查找对应的能力规格。
 * 遍历所有激活能力寻找具有匹配标签的能力规格。
 *
 * @param AbilityTag 能力标签
 * @return 返回指向能力规格的指针或nullptr
 */
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

/**
 * 服务端实现：消耗法术点来提升能力等级或解锁能力。
 * 根据当前能力状态决定是解锁还是升级。
 *
 * @param AbilityTag 能力标签
 */
void UAuraAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
		FGameplayTag Status = GetStatusFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level += 1;
		}
		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec);
	}
}

/**
 * 获取指定能力的描述文本。
 * 若能力存在则返回当前等级及下一级描述；否则返回锁定描述。
 *
 * @param AbilityTag 能力标签
 * @param OutDescription 输出参数，当前等级描述字符串
 * @param OutNextLevelDescription 输出参数，下一等级描述字符串
 * @return 成功获取描述则返回true，否则false
 */
bool UAuraAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if(UAuraGameplayAbility* AuraAbility = Cast<UAuraGameplayAbility>(AbilitySpec->Ability))
		{
			OutDescription = AuraAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = AuraAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FAuraGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	}
	else
	{
		OutDescription = UAuraGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

/**
 * 服务端实现：装备指定能力至特定槽位。
 * 处理已有能力替换、被动技能激活等复杂情况。
 *
 * @param AbilityTag 要装备的能力标签
 * @param Slot 目标槽位标签
 */
void UAuraAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Slot)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
		const FGameplayTag& PrevSlot = GetInputTagFromSpec(*AbilitySpec);
		const FGameplayTag& Status = GetStatusFromSpec(*AbilitySpec);

		const bool bStatusValid = Status == GameplayTags.Abilities_Status_Equipped || Status == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{

			// Handle activation/deactivation for passive abilities

			if (!SlotIsEmpty(Slot)) // There is an ability in this slot already. Deactivate and clear its slot.
			{
				FGameplayAbilitySpec* SpecWithSlot = GetSpecWithSlot(Slot);
				if (SpecWithSlot)
				{
					// is that ability the same as this ability? If so, we can return early.
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
						return;
					}

					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false);
						DeactivatePassiveAbility.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearSlot(SpecWithSlot);
				}
			}

			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't yet have a slot (it's not active)
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					MulticastActivatePassiveEffect(AbilityTag, true);
				}
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GetStatusFromSpec(*AbilitySpec));
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
			}
			AssignSlotToAbility(*AbilitySpec, Slot);
			MarkAbilitySpecDirty(*AbilitySpec);
		}
		ClientEquipAbility(AbilityTag, GameplayTags.Abilities_Status_Equipped, Slot, PrevSlot);
	}
}

/**
 * 多播实现：激活或停用被动特效。
 * 广播激活/停用事件供其他模块响应。
 *
 * @param AbilityTag 能力标签
 * @param bActivate 是否激活标志
 */
void UAuraAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}


/**
 * 判断指定能力是否为被动能力。
 * 通过查询能力信息表中的类型字段进行判定。
 *
 * @param Spec 能力规格引用
 * @return 是被动能力返回true，否则false
 */
bool UAuraAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& Spec) const
{
	const UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
	const FAuraAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Type_Passive);
}

/**
 * 判断能力是否已经分配了任意槽位。
 * 检查动态源标签中是否存在"InputTag"相关标签。
 *
 * @param Spec 能力规格引用
 * @return 已分配槽位返回true，否则false
 */
bool UAuraAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& Spec)
{
	return Spec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("Input")));
}

/**
 * 将指定槽位分配给能力。
 * 清除旧槽位后添加新槽位标签。
 *
 * @param Spec 能力规格引用
 * @param Slot 新槽位标签
 */
void UAuraAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	ClearSlot(&Spec);
	Spec.GetDynamicSpecSourceTags().AddTag(Slot);
}

/**
 * 清除能力上的槽位标签。
 * 移除当前能力所绑定的输入槽位标签。
 *
 * @param Spec 能力规格指针
 */
void UAuraAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

/**
 * 客户端实现：通知能力装备成功。
 * 广播装备事件供UI或其他模块更新显示。
 *
 * @param AbilityTag 能力标签
 * @param Status 当前状态标签
 * @param Slot 新槽位标签
 * @param PreviousSlot 原始槽位标签
 */
void UAuraAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	AbilityEquipped.Broadcast(AbilityTag, Status, Slot, PreviousSlot);
}

/**
 * 判断指定槽位是否为空闲状态。
 * 遍历所有能力查看是否有占用该槽位的情况。
 *
 * @param Slot 待检测槽位标签
 * @return 槽位空闲返回true，否则false
 */
bool UAuraAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLoc(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, Slot))
		{
			return false;
		}
	}
	return true;
}

/**
 * 判断指定能力是否绑定了某个槽位。
 * 检查能力动态源标签中是否精确包含目标槽位标签。
 *
 * @param Spec 能力规格引用
 * @param Slot 槽位标签
 * @return 匹配返回true，否则false
 */
bool UAuraAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot)
{
	return Spec.GetDynamicSpecSourceTags().HasTagExact(Slot);
}

/**
 * 根据槽位标签查找对应的能力规格。
 * 遍历所有能力寻找第一个匹配槽位的能力规格。
 *
 * @param Slot 槽位标签
 * @return 找到返回能力规格指针，否则nullptr
 */
FGameplayAbilitySpec* UAuraAbilitySystemComponent::GetSpecWithSlot(const FGameplayTag& Slot)
{
	FScopedAbilityListLock ActiveScopeLock(*this);
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(Slot))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

/**
 * 客户端实现：更新能力状态变化。
 * 广播状态变更事件供外部监听器处理。
 *
 * @param AbilityTag 能力标签
 * @param StatusTag 状态标签
 * @param AbilityLevel 能力等级
 */
void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
                                                                           const FGameplayTag& StatusTag, int32 AbilityLevel)
{
	AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}

/**
 * 根据能力标签获取其当前状态。
 * 内部调用GetSpecFromAbilityTag和GetStatusFromSpec完成转换。
 *
 * @param AbilityTag 能力标签
 * @return 返回能力状态标签或空标签
 */
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusFromSpec(*Spec);
	}
	return FGameplayTag();
}

/**
 * 根据能力标签获取其所在槽位。
 * 内部调用GetSpecFromAbilityTag和GetInputTagFromSpec完成转换。
 *
 * @param AbilityTag 能力标签
 * @return 返回槽位标签或空标签
 */
FGameplayTag UAuraAbilitySystemComponent::GetSlotFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*Spec);
	}
	return FGameplayTag();
}
