// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class UNiagaraComponent;

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	//--------- CombatInterface --------------
	virtual int32 GetPlayerLevel_Implementation() const override;
	//--------- end CombatInterface --------------

	//--------- PlayerInterface --------------
	virtual int32 GetXP_Implementation() const override;
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual void LevelUp_Implementation() override;
	virtual void AddToPlayerLevel_Implementation(int32 LevelToAdd) override;
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;

	//--------- end PlayerInterface --------------
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

protected:
	virtual void InitAbilityActorInfo() override;

private:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;
};
