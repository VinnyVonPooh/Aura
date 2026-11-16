// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraFireBolt.h"

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

	const int32 NumProjectiles = 2; // GetNumProjectiles(GetAbilityLevel());

	const FVector Forward = Rotation.Vector();
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-ProjectileSpread / 2.f, FVector::UpVector);
	const FVector RightOfSpread = Forward.RotateAngleAxis(ProjectileSpread / 2.f, FVector::UpVector);
	
	if (NumProjectiles > 1) {

		const float DeltaSpread = ProjectileSpread / (NumProjectiles - 1);
		for (int32 i = 0; i < NumProjectiles; ++i) {
			
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);

			UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + Direction * 100.f, 5,
												 FLinearColor::Red, 60, 1);
		}
	} 

	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + Forward * 200.f, 5,
										 FLinearColor::White, 60, 3);

	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + LeftOfSpread * 200.f, 5,
										 FLinearColor::Green, 60, 3);

	UKismetSystemLibrary::DrawDebugArrow(GetAvatarActorFromActorInfo(), SocketLocation, SocketLocation + RightOfSpread * 200.f, 5,
										 FLinearColor::Green, 60, 3);
}

int32 UAuraFireBolt::GetNumProjectiles(int32 Level) const
{
	return FMath::Min(MaxNumProjectiles, Level);
}