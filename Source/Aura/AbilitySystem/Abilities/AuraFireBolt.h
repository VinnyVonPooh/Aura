// Copyright (c) MarmoDrake. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AuraProjectileSpell.h"
#include "AuraFireBolt.generated.h"

UCLASS()
class AURA_API UAuraFireBolt : public UAuraProjectileSpell
{
	GENERATED_BODY()
public:
	virtual FString GetDescription(int32 Level) const override;
	virtual FString GetNextLevelDescription(int32 Level) const override;
};
