// Copyright (c) MarmoDrake. All Rights Reserved.

#include "AuraProjectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Aura/Aura.h"
#include "Aura/AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AAuraProjectile::AAuraProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AAuraProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereOverlap);

	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent());

	SetLifeSpan(LifeSpan);
}

void AAuraProjectile::Destroyed()
{
	if (LoopingSoundComponent) {
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}

	if (!bHit && !HasAuthority()) {
		OnHit();
	}
	Super::Destroyed();
}

void AAuraProjectile::OnHit()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
	if (LoopingSoundComponent) {
		LoopingSoundComponent->Stop();
		LoopingSoundComponent->DestroyComponent();
	}
	bHit = true;
}

void AAuraProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
									  int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GetOwner() == OtherActor || GetInstigator() == OtherActor) {
		return;
	}

	if (!DamageEffectParams.SourceAbilitySystemComponent) {
		return;
	}

	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	if (SourceAvatarActor == OtherActor) {
		return;
	}
	
	if (!UAuraAbilitySystemLibrary::IsNotFriend(SourceAvatarActor, OtherActor)) {
		return;
	}

	if (!bHit) {
		OnHit();
	}

	if (HasAuthority()) {

		// Применение на OtherActor эффекта, дамажищего при оверлапе
		// Применяем только на сервере
		if (auto* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor)) {

			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;

			if (FMath::RandRange(1, 100) < DamageEffectParams.KnockbackChance) {
				FRotator Rotator = GetActorRotation();
				Rotator.Pitch = 15.f;
				const FVector KnockbackDirection = Rotator.Vector();
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
				DamageEffectParams.KnockbackImpulse = KnockbackForce;
			}

			// устанавливаем тут, таргет стал известен, до этого проставляли nullptr
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			UAuraAbilitySystemLibrary::ApplyDamageEffect(DamageEffectParams);
		}
		// разрушаем на сервере
		Destroy();
	} else {
		// на клиенте
		bHit = true;
	}
}
