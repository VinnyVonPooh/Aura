// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AuraCharacterBase.h"
#include "Aura/Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	AAuraCharacter();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	//--------- CombatInterface --------------
	virtual int32 GetPlayerLevel() const override;
	//--------- end CombatInterface --------------


	//--------- PlayerInterface --------------
	virtual void AddToXP_Implementation(int32 InXP) override;

	//--------- end PlayerInterface --------------
protected:
	virtual void InitAbilityActorInfo() override;

private:
	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* Camera;
};
