// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraProjectileSpell.h"

#include "Aura/Actor/AuraProjectile.h"
#include "Aura/Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
										   const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	const bool bIsServer = HasAuthority(&ActivationInfo);
	if (!bIsServer) {
		return;
	}

	if (const auto* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo())) {
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);

		auto* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(),
																		   Cast<APawn>(GetOwningActorFromActorInfo()),
																		   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);


		Projectile->FinishSpawning(SpawnTransform);
	}
}