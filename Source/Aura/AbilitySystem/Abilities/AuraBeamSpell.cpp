// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraBeamSpell.h"

#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.bBlockingHit) {
		MouseHitLocation = HitResult.ImpactPoint;
		MouseHitActor = HitResult.GetActor();
	} else {
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, GetCurrentActivationInfo(), true);
	}
}

void UAuraBeamSpell::StoreOwnerPlayerVariables()
{
	if (CurrentActorInfo) {
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>()) {
		if (const auto* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter)) {

			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));

			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);

			FHitResult HitResult;

			UKismetSystemLibrary::SphereTraceSingle(OwnerCharacter, SocketLocation, BeamTargetLocation, 10.f, TraceTypeQuery1, false,
													ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);

			if (HitResult.bBlockingHit) {
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
}