// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraFireBolt.h"

#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Actor/AuraProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FString UAuraFireBolt::GetDescription(int32 Level) const
{
	const int ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	if (Level == 1) {
		return FString::Printf(TEXT("<Title>FIRE BOLT</>\n\n"
									"<Small>Level: </>"
									"<Level>%d</>\n"

									"<Small>ManaCost: </>"
									"<ManaCost>%.1f</>\n"

									"<Small>Cooldown: </>"
									"<Cooldown>%.1f</>\n"

									"<Default>Launches a bolt of fire, exploding on impact and dealing: </>"
									"<Damage>%d</>"
									"<Default> fire damage with a chance to burn</>"),
							   Level, ManaCost, Cooldown, ScaledDamage);
	}
	return FString::Printf(TEXT("<Title>FIRE BOLT</>\n\n"
								"<Small>Level: </>"
								"<Level>%d</>\n"

								"<Small>ManaCost: </>"
								"<ManaCost>%.1f</>\n"

								"<Small>Cooldown: </>"
								"<Cooldown>%.1f</>\n"

								"<Default>Launches %d bolts of fire, exploding on impact and dealing: </>"
								"<Damage>%d</>"
								"<Default> fire damage with a chance to burn</>"),
						   Level, ManaCost, Cooldown, GetNumProjectiles(Level), ScaledDamage);
}

FString UAuraFireBolt::GetNextLevelDescription(int32 Level) const
{
	const int ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);

	return FString::Printf(TEXT("<Title>NEXT LEVEL</>\n\n"
								"<Small>Level: </>"
								"<Level>%d</>\n"

								"<Small>ManaCost: </>"
								"<ManaCost>%.1f</>\n"

								"<Small>Cooldown: </>"
								"<Cooldown>%.1f</>\n"

								"<Default>Launches %d bolts of fire, exploding on impact and dealing: </>"
								"<Damage>%d</>"
								"<Default> fire damage with a chance to burn</>"),
						   Level, ManaCost, Cooldown, GetNumProjectiles(Level), ScaledDamage);
}

void UAuraFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, AActor* HomingTarget,
									 bool bOverridePitch, float PitchOverride)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) {
		return;
	}

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);

	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	Rotation.Pitch = 0.f;
	// приподнять полёт снаряда для маленьких монстров, чтобы не летелов землю
	if (bOverridePitch) {
		Rotation.Pitch = PitchOverride;
	}

	const FVector Forward = Rotation.Vector();

	const int32 NumProjectiles = GetNumProjectiles(GetAbilityLevel());
	auto Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);

	for (auto& Rot : Rotations) {

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());

		auto* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(),
																		   Cast<APawn>(GetAvatarActorFromActorInfo()),
																		   ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		if (HomingTarget && HomingTarget->Implements<UCombatInterface>()) {
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		} else {
			// сохраняем по сильной ссылке, чтобы после выхода из функции SceneComponent в слабой ссылке у ProjectileMovement не удалился
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}
		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::RandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectile;
		Projectile->FinishSpawning(SpawnTransform);
	}
}

int32 UAuraFireBolt::GetNumProjectiles(int32 Level) const
{
	return FMath::Min(MaxNumProjectiles, Level);
}