// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"
#include <AttributeSet.h>

#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Interface/CombatInterface.h"
#include "Aure_B_Character.generated.h"


class UNiagaraSystem;
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
    // 构造与初始化
    AAure_B_Character();

    // Combat接口实现
    UPROPERTY(EditAnywhere, Category="Combat")
    TArray<FTaggedMontage> AttackMontages;

    virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
    virtual UAnimMontage* GetCombatMontage_Implementation() override;
    virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
    virtual bool IsDead_Implementation() const override;
    virtual AActor* GetAvatar_Implementation() override;
    virtual UNiagaraSystem* GetBloodEffect_Implementation() const override;
    virtual FTaggedMontage GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag) override;
    virtual int32 GetMinionCount_Implementation() override;
    virtual void IncremenetMinionCount_Implementation(int32 Amount) override;
    virtual ECharacterClass GetCharacterClass_Implementation() override;

    // 能力系统接口实现
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    UAttributeSet* GetAttributeSet() const { return AttributeSet; }

    // 死亡处理
    virtual void Die() override;

    UFUNCTION(NetMulticast, Reliable)
    virtual void MulticastHandleDeath();

protected:
    // 生命周期与初始化
    virtual void BeginPlay() override;
    virtual void InitAbilityActorInfo();
    virtual void InitDefaultAttributes() const;
    void ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const;
    void AddCharacterAbilities();

    
    UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Character Class Defaults")
    ECharacterClass CharacterClass = ECharacterClass::Warrior;
    
    // 溶解特效
    void Dissolve();

    UFUNCTION(BlueprintImplementableEvent)
    void StartDissolveTimeline(UMaterialInstanceDynamic* MaterialInstanceDynamic);

    UFUNCTION(BlueprintImplementableEvent)
    void StartWeaponDissolveTimeline(UMaterialInstanceDynamic* MaterialInstanceDynamic);

    // Combat配置
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TObjectPtr<USkeletalMeshComponent> Weapon;

    UPROPERTY(EditAnywhere, Category = "Combat")
    FName WeaponTipSocketName;

    UPROPERTY(EditAnywhere, Category = "Combat")
    FName LeftHandSocketName;

    UPROPERTY(EditAnywhere, Category = "Combat")
    FName RightHandSocketName;

    UPROPERTY(EditAnywhere, Category = "Combat")
    FName TailSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    UNiagaraSystem* BloodEffect;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    USoundBase* DeathSound;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Combat")
    TObjectPtr<UAnimMontage> HitReactMontage;

    // 能力系统与属性集
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    UPROPERTY()
    TObjectPtr<UAttributeSet> AttributeSet;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TSubclassOf<UGameplayEffect> DefaultPrimaryAttributes;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TSubclassOf<UGameplayEffect> DefaultSecondaryAttributes;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TSubclassOf<UGameplayEffect> DefaultInitVitalAttributes;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Attributes")
    TArray<TSubclassOf<UGameplayAbility>> StartupPassiveAbilities;
    
    // 材质与视觉效果
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UMaterialInstance> DissolveMaterialInstance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UMaterialInstance> WeaponDissolveMaterialInstance;

    // 状态标志
    bool bIsDead = false;

    int32 MinionCount = 0;
};

