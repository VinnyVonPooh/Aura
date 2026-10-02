// Copyright (c) MarmoDrake. All Rights Reserved.


#include "TargetDataUnderMouse.h"

UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MYObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MYObj;
}

void UTargetDataUnderMouse::Activate()
{
	Super::Activate();

	const auto* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	ValidData.Broadcast(CursorHit.Location);
}