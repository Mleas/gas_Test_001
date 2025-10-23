// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Aure_B_Character.h"
#include "Interface/EnemyInterface.h"
#include "Aure_Enemy.generated.h"

/**
 * 
 */
UCLASS()
class AC__API AAure_Enemy : public AAure_B_Character, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
    AAure_Enemy();


    // IEnemyInterface
    virtual void HighlightActor() override;
    virtual void UnHighlightActor() override;

	//Combat Interface
	virtual int32 GetPlayerLevel() override;
	
protected:
    virtual void BeginPlay() override;

	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category = "Character Class Defaults")
	int32 Level = 1;

private:

	
};
