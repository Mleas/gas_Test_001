// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"
#include "AuraAbilitySystemLibrary.generated.h"


class UAttributeMenuWidgetController;

/**
 * UAuraAbilitySystemLibrary 是一个蓝图函数库类，用于提供与角色能力系统相关的通用功能。
 * 包括获取UI控制器、初始化默认属性、赋予初始技能以及处理游戏效果上下文中的暴击和格挡状态等功能。
 */
UCLASS()
class AC__API UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:


	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD);
	/**
	 * 获取覆盖层UI控制器实例。
	 * @param WorldContextObject 世界上下文对象，用于获取游戏世界信息。
	 * @return 返回一个UOverlayWidgetController指针，用于控制覆盖层UI逻辑。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/**
	 * 获取属性菜单UI控制器实例。
	 * @param WorldContextObject 世界上下文对象，用于获取游戏世界信息。
	 * @return 返回一个UAttributeMenuWidgetController指针，用于控制属性菜单UI逻辑。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);


	/**
	 * 初始化指定角色类的默认属性。
	 * @param WorldContextObject 世界上下文对象。
	 * @param CharacterClass 角色类别，决定使用哪一组默认属性配置。
	 * @param Level 当前等级，影响属性数值计算。
	 * @param ASC 能力系统组件，用于应用属性集。
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void InitDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	/**
	 * 给予角色初始技能。
	 * @param WorldContextObject 世界上下文对象。
	 * @param ASC 能力系统组件，用于授予技能。
	 * @param CharacterClass 角色类别，决定授予哪些初始技能。
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);

	/**
	 * 获取角色类信息数据资产。
	 * @param WorldContextObject 世界上下文对象。
	 * @return 返回指向UCharacterClassInfo的数据资产指针。
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	/**
	 * 判断该效果是否为格挡命中。
	 * @param EffectContextHandle 效果上下文句柄，包含命中相关信息。
	 * @return 如果是格挡命中则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 判断该效果是否为暴击命中。
	 * @param EffectContextHandle 效果上下文句柄，包含命中相关信息。
	 * @return 如果是暴击命中则返回true，否则返回false。
	 */
	UFUNCTION(BlueprintPure, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	/**
	 * 设置效果是否为格挡命中。
	 * @param EffectContextHandle 效果上下文句柄，将被修改以标记格挡状态。
	 * @param bInIsBlockedHit 标记是否为格挡命中的布尔值。
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	/**
	 * 设置效果是否为暴击命中。
	 * @param EffectContextHandle 效果上下文句柄，将被修改以标记暴击状态。
	 * @param bInCriticalHit 标记是否为暴击命中的布尔值。
	 */
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInCriticalHit);

	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|GameMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintCallable,BlueprintPure, Category="AuraAbilitySystemLibrary|GameMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	
	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass,
	                                  int32 CharacterLevel);

	
	UFUNCTION(BlueprintCallable, Category="AuraAbilitySystemLibrary|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);


};
