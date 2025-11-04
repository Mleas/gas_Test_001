// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include <AttributeSet.h>

#include "Abilities/GameplayAbility.h"
#include "Interface/CombatInterface.h"
#include "Aure_B_Character.generated.h"


class UGameplayEffect;
class UAbilitySystemComponent;
class UAuraAttributeSet;
/**
 * @brief 抽象角色类，继承自ACharacter并实现IAbilitySystemInterface和ICombatInterface接口。
 *        提供基础的角色能力系统支持、战斗相关功能以及属性集管理。
 */
UCLASS(Abstract)
class AC__API AAure_B_Character : public ACharacter, public IAbilitySystemInterface, public ICombatInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief 构造函数，初始化默认属性值。
	 */
	AAure_B_Character();

	/**
	 * @brief 获取当前角色的能力系统组件（Ability System Component）。
	 * 
	 * @return 返回指向UAbilitySystemComponent的指针。
	 */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/**
	 * @brief 获取当前角色的属性集。
	 * 
	 * @return 返回指向UAttributeSet的指针。
	 */
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/**
	 * @brief 实现CombatInterface中的GetCombatMontage方法，用于获取战斗动画蒙太奇。
	 * 
	 * @return 返回指向UAnimMontage的指针。
	 */
	virtual UAnimMontage* GetCombatMontage_Implementation() override;

	/**
	 * @brief 角色死亡处理逻辑。
	 */
	virtual void Die() override;

	/**
	 * @brief 多播网络事件：处理角色死亡后的表现效果。
	 */
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath();

	/**
	 * @brief 实现CombatInterface中的GetCombatSocketLocation方法，返回武器尖端位置。
	 * 
	 * @return 返回FVector表示的位置坐标。
	 */
	virtual FVector GetCombatSocketLocation_Implementation() override;

	/**
	 * @brief 判断角色是否已死亡。
	 * 
	 * @return true 表示角色已经死亡；false 表示角色仍然存活。
	 */
	virtual bool IsDead_Implementation() const override;

	/**
	 * @brief 获取当前角色作为Avatar的对象。
	 * 
	 * @return 返回指向AActor的指针。
	 */
	virtual AActor* GetAvatar_Implementation() override;

protected:
	/**
	 * @brief 游戏开始或对象生成时调用，执行初始化操作。
	 */
	virtual void BeginPlay() override;

	/**
	 * @brief 初始化角色的能力信息，包括绑定ASC到Owner等。
	 */
	virtual void InitAbilityActorInfo();

	/**
	 * @brief 初始化角色的默认属性集。
	 */
	virtual void InitDefaultAttributes() const;

	/**
	 * @brief 应用指定的游戏效果到自身。
	 * 
	 * @param GameplayEffectClass 要应用的效果类。
	 * @param Level 效果等级。
	 */
	void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;

	/**
	 * @brief 添加初始拥有的技能能力。
	 */
	void AddCharacterAbilities();

	/**
	 * @brief 启动溶解视觉特效流程。
	 */
	void Dissolve();

	/**
	 * @brief 蓝图可实现事件：启动角色模型的溶解时间轴。
	 * 
	 * @param MaterialInstanceDynamic 动态材质实例。
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartDissolveTimeline(UMaterialInstanceDynamic* MaterialInstanceDynamic);

	/**
	 * @brief 蓝图可实现事件：启动武器模型的溶解时间轴。
	 * 
	 * @param MaterialInstanceDynamic 动态材质实例。
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* MaterialInstanceDynamic);

	/** 武器网格组件 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	TObjectPtr<USkeletalMeshComponent> Weapon;

	/** 武器尖端插槽名称 */
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponTipSocketName;

	/** 角色能力系统组件 */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 属性集合 */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

	/** 默认主属性配置 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

	/** 默认次级属性配置 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

	/** 默认初始生命值等重要属性配置 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TSubclassOf<UGameplayEffect> DefaultInitVitalAttributes;

	/** 溶解材质实例 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

	/** 武器溶解材质实例 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

	/** 初始拥有的能力列表 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	/** 受击反应动画蒙太奇 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
	TObjectPtr<UAnimMontage> HitReactMontage;

	/** 是否已死亡标志位 */
	bool bIsDead = false;
};

