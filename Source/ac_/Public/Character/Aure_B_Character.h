// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Aure_B_Character.generated.h"

UCLASS(Abstract)
class AC__API AAure_B_Character : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAure_B_Character();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
