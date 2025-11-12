// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Aure_B_Character.h"
#include "Interface/PlayerInterface.h"
#include "Aure_Character.generated.h"

/**
 * 
 */

class UNiagaraComponent;
class USpringArmComponent;
class UCameraComponent;
DECLARE_DELEGATE_RetVal_OneParam(int32, FONTest, int32);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FONTestDyn, int32, test);

UCLASS()
class AC__API AAure_Character : public AAure_B_Character, public IPlayerInterface
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
	virtual int32 GetPlayerLevel_Implementation() override;

	//Player Interface
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoints_Implementation() const override;
/*	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial) override;
	virtual void HideMagicCircle_Implementation() override;
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;

*/
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;


private:
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;

	AAuraPlayerState* GetAuraPS() const;
};
