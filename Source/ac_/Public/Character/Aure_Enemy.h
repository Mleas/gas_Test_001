// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Character/Aure_B_Character.h"
#include "Interface/EnemyInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "Aure_Enemy.generated.h"


class AAuraAIController;
class UBehaviorTree;
class UWidgetComponent;

/**
 * 
 */
UCLASS()
class AC__API AAure_Enemy : public AAure_B_Character, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
    AAure_Enemy();
	virtual void PossessedBy(AController* NewController) override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
    // IEnemyInterface
    virtual void HighlightActor() override;
    virtual void UnHighlightActor() override;

	//Combat Interface
	virtual int32 GetPlayerLevel() override;
	virtual void Die() override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;

	void HitReatTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly,Category="Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadOnly,Category="Combat")
	float BaseWalkSpeed = 250.f;

	UPROPERTY(BlueprintReadOnly,Category="Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadOnly,Category="Combat")
	AActor* CombatTarget = nullptr;
protected:
    virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;
	virtual void InitDefaultAttributes() const override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
	
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere,Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
	

	UPROPERTY(EditAnywhere,Category="AI")
	TObjectPtr<AAuraAIController> AuraAIController;


private:

	
};
