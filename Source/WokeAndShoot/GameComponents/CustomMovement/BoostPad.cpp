// Fill out your copyright notice in the Description page of Project Settings.


#include "BoostPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "../Projectiles/WokeAndShootProjectile.h"
#include "../Character/WokeAndShootCharacter.h"
#include "DrawDebugHelpers.h"
#include "MyCharacterMovementComponent.h"

ABoostPad::ABoostPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("BoostPad");

	RootComponent = CollisionComp;
	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoostPadMesh"));
	PadMesh->AttachTo(CollisionComp);

}

void ABoostPad::BoostPlayers(AWokeAndShootCharacter* Initiator) 
{
	//ApplyBoost to Local
	ApplyBoost(Initiator);
}

void ABoostPad::BeginPlay()
{
	Super::BeginPlay();
}

void ABoostPad::InitiateCooldown(AWokeAndShootCharacter* HitActor) 
{
	FTimerHandle TH_CooldownTimer;
	CooldownList.AddUnique(HitActor);
	TimerHandles.AddUnique(TH_CooldownTimer);

	GetWorld()->GetTimerManager().SetTimer(TH_CooldownTimer,this,&ABoostPad::RemoveActorCD,BoostPadCooldown);	
}

void ABoostPad::RemoveActorCD() 
{
	CooldownList.Pop();
}

bool ABoostPad::WithinConeRange(FVector& PadLocation, FVector& HitActorLocation) 
{
	//Calculate cone range
	FVector FrontFacingVector = PadLocation + GetActorRotation().RotateVector(FVector(0,100,0));

	FVector ForwardFacingDirection = FrontFacingVector - PadLocation;
	ForwardFacingDirection.Normalize();

	FVector TargetFacingDirection = HitActorLocation - PadLocation;
	TargetFacingDirection.Normalize();

	//Check if Actor is within cone
	float AngleToFront = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ForwardFacingDirection, TargetFacingDirection)));

	if(AngleToFront > 80.f) {return false;}
	else {return true;}
}

FVector ABoostPad::GetImpulseDirection(FVector& ActorLocation) 
{
	//Calculate Impulse direction+amount
	FVector ImpulseDirection = (ActorLocation + HeightOffset ) - GetActorLocation();
	ImpulseDirection.Normalize();
	ImpulseDirection*= BoostAmount;
		
	return ImpulseDirection;
}

void ABoostPad::ApplyBoost(AWokeAndShootCharacter* Initiator) 
{
	TArray<FHitResult> HitResults;
	FVector SweepStart = GetActorLocation();
	FVector SweepEnd = GetActorLocation();

	FCollisionShape MyColSphere = FCollisionShape::MakeSphere(800.0f);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool isHit = GetWorld()->SweepMultiByChannel(HitResults, SweepStart, SweepEnd, FQuat::Identity, ECC_GameTraceChannel3, MyColSphere, QueryParams);
	if (isHit)
	{
		for(FHitResult HitResult : HitResults)
		{
			// UE_LOG(LogTemp,Warning,TEXT("Actor Hit: %s"),*HitResult.GetActor()->GetName());
			AWokeAndShootCharacter* HitActor = Cast<AWokeAndShootCharacter>(HitResult.GetActor());
			if (HitActor != nullptr)
			{
				
				FVector ActorLocation = HitActor->GetActorLocation();

				if(CooldownList.Contains(HitActor)){continue;}
				if(!WithinConeRange(SweepStart, ActorLocation)){continue;}

				FVector ImpulseDirection = GetImpulseDirection(ActorLocation);
				
				if(HitActor->GetLocalRole() == ROLE_SimulatedProxy)
				{
					GLog->Log("BOOSTING SIMULATED PROXY");
				}
				if(!HitActor->IsLocallyControlled() && Initiator->IsLocallyControlled() && !HasAuthority())
				{
					continue;
				}
				//Apply Impulse
				HitActor->DirectionalImpulse(ImpulseDirection);
				//Add Cooldown
				InitiateCooldown(HitActor);


				//Debug Line for Forward Facing Line
				// DrawDebugLine(GetWorld(),SweepStart ,(SweepStart + GetActorRotation().RotateVector(FVector(0,100,0))), FColor::Red, true);
				// GLog->Log("Character Hit");
			}
		}
	}
	// DrawDebugSphere(GetWorld(), GetActorLocation(), MyColSphere.GetSphereRadius(), 50, FColor::Purple, true);
	// GLog->Log("Fired");
}

