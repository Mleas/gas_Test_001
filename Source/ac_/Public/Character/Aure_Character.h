// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Aure_B_Character.h"
#include "Aure_Character.generated.h"

/**
 * 
 */

DECLARE_DELEGATE_RetVal_OneParam(int32, FONTest, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONTestDyn, int32, test);

UCLASS()
class AC__API AAure_Character : public AAure_B_Character
{
	GENERATED_BODY()
public:
	AAure_Character();

	FONTest OnTest;

    UPROPERTY(BlueprintReadWrite, BlueprintAssignable)
	FONTestDyn OnTestDyn;

	virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayEffect>> DefaultPassiveGameplayEffect;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	//Combat Interface
	virtual int32 GetPlayerLevel() override;

private:
	virtual void InitAbilityActorInfo() override;


	
};
