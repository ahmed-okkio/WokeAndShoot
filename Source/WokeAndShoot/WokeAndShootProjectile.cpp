// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

AWokeAndShootProjectile::AWokeAndShootProjectile() 
{
	
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	CollisionComp->OnComponentHit.AddDynamic(this, &AWokeAndShootProjectile::OnHit);		// set up a notification for when this component hits something blocking

	// Players can't walk on it
	// CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	// CollisionComp->CanCharacterStepUpOn = ECB_Yes;

	// Set as root component
	RootComponent = CollisionComp;
	ProjectileStaticMesh =  CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	ProjectileStaticMesh->AttachTo(RootComponent);
	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 10000.f;
	// ProjectileMovement->AddForce(GetActorRotation().Vector() * 350000.f);
	// ProjectileMovement->MaxSpeed = 55000.f;
	// ProjectileMovement->bRotationFollowsVelocity = true;
	FTimerHandle TracerUpdateTH;
	if(GetWorld() != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SPEED UPDATE TIMER"));
		// GetWorld()->GetTimerManager().SetTimer(TracerUpdateTH, this, &AWokeAndShootProjectile::UpdateSpeed, 0.005, false);
	}
	
	// Die after 3 seconds by default
	InitialLifeSpan = 3.0f;
}

void AWokeAndShootProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Only add impulse and destroy projectile if we hit a physics
	if ((OtherActor != nullptr) && (OtherActor != this) && (OtherComp != nullptr) /*&& OtherComp->IsSimulatingPhysics()*/)
	{
		// OtherComp->AddImpulseAtLocation(GetVelocity() * 100.0f, GetActorLocation());
		// if(GetOwner() != nullptr)
		// {
		// 	UE_LOG(LogTemp,Warning,TEXT("%s"),*GetOwner()->GetName());
		// }
		// else
		// {
		// 	GLog->Log("No Owner");
		// }
		Destroy();
	}
}

AActor* AWokeAndShootProjectile::GetOwnerActor() 
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor != nullptr)
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
	// ProjectileMovement->Velocity = GetActorRotation().Vector() * 45000.f;
	ProjectileMovement->AddForce(GetActorRotation().Vector() * 45000.f);
	// CollisionComp->ComponentVelocity = 25000.f;
	UE_LOG(LogTemp, Warning, TEXT("UPDATE SPEED"));

}
