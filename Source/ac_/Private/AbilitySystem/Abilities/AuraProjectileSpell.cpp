// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AuraGameplayTags.h"
#include "Actor/AuraProjectile.h"
#include "Interface/CombatInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                           const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                           const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	
	
}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLoction, const FGameplayTag& SocketTag)
{
    // 检查是否是服务器（已注释掉的旧逻辑）
    // const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    // if (!bIsServer) return;

    // 如果当前激活信息没有权限（不是服务器），则直接返回
    if (!HasAuthority(&CurrentActivationInfo)) return;
    
    // 获取角色的战斗接口（用于获取技能释放位置等）
    ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
    
    if (CombatInterface)
    {
        // 获取技能释放的起始位置（通常是角色的手或武器挂点）
        FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
            GetAvatarActorFromActorInfo(),
            SocketTag);
        SocketLocation.Z -= 10.0f;

        // 计算从释放点到目标点的旋转方向
        FRotator Rotator = (ProjectileTargetLoction - SocketLocation).Rotation();
        Rotator.Pitch = 0.0f; // 将俯仰角设为0，防止抛物线或上下偏移

        // 构造一个用于生成投射物的变换（位置 + 旋转）
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SocketLocation);
        SpawnTransform.SetRotation(Rotator.Quaternion());

        // 延迟生成投射物（AAuraProjectile），设置生成参数
        AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
            ProjectileClass, 
            SpawnTransform, 
            GetOwningActorFromActorInfo(),
            Cast<APawn>(GetOwningActorFromActorInfo()), 
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        // 获取技能的能力系统组件（ASC）
        const UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo());

        FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
        EffectContextHandle.AddSourceObject(GetAvatarActorFromActorInfo());
        
        // 创建一个用于伤害的 GameplayEffectSpecHandle（效果规格句柄）
        const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
            DamageEffectClass,
            GetAbilityLevel(),
            EffectContextHandle);

        // 获取游戏标签（用于标识伤害类型）
        FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

     
            // 根据技能等级获取缩放后的伤害值
            const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());

            // 将伤害值通过标签设置到 SpecHandle 中（SetByCaller）
            UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            SpecHandle, 
            DamageType,
            ScaledDamage);
            
      
        
        
        // 将伤害效果规格句柄赋值给投射物
        Projectile->DamageEffectSpecParams = SpecHandle;

        // 完成投射物的生成
        Projectile->FinishSpawning(SpawnTransform);
    }
}
