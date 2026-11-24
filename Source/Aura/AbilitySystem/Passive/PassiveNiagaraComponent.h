// Copyright (c) MarmoDrake. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "PassiveNiagaraComponent.generated.h"

UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()

public:
	UPassiveNiagaraComponent();
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;

protected:
	void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);

	UFUNCTION()
	void OnOwnerDeath(AActor* DeadActor);
};
