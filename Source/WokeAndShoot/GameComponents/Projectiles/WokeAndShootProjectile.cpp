// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "../Character/WokeAndShootCharacter.h"
#include "../Character/CharacterComponenets/HealthComponent.h"

AWokeAndShootProjectile::AWokeAndShootProjectile() 
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->MoveIgnoreActors.Add(GetOwnerActor());
	CollisionComp->OnComponentHit.AddDynamic(this, &AWokeAndShootProjectile::OnHit);	

	RootComponent = CollisionComp;

	ProjectileStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ProjectileStaticMesh->AttachTo(RootComponent);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->bRotationFollowsVelocity = true;
	
	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AWokeAndShootProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) /*&& OtherComp->IsSimulatingPhysics()*/)
	{
		Destroy();
	}
}

AActor* AWokeAndShootProjectile::GetOwnerActor() 
{
	
	if (AActor* OwnerActor = GetOwner())
	{
		return GetOwner();
	}
	else
	{
		return nullptr;
	}

}

UStaticMeshComponent* AWokeAndShootProjectile::GetProjectileMesh() 
{
	 return ProjectileStaticMesh;
}

void AWokeAndShootProjectile::UpdateSpeed() 
{
	ProjectileMovement->AddForce(GetActorRotation().Vector() * 45000.f);
	UE_LOG(LogTemp, Warning, TEXT("UPDATE SPEED"));

}
