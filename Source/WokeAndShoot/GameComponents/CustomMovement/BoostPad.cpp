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

	DefaultMaterial = CreateDefaultSubobject<UMaterialInstance>(TEXT("DefaultMaterial"));
	PrimedMaterial = CreateDefaultSubobject<UMaterialInstance>(TEXT("PrimedMaterial"));
	DetonatedMaterial = CreateDefaultSubobject<UMaterialInstance>(TEXT("DetonatedMaterial"));

}

void ABoostPad::BeginPlay()
{
	Super::BeginPlay();
}

void ABoostPad::InitiateCooldown(AWokeAndShootCharacter* HitActor) 
{
	FTimerHandle TH_CooldownTimer;
	CooldownList.AddUnique(HitActor);
	// TimerHandles.AddUnique(TH_CooldownTimer);

	GetWorld()->GetTimerManager().SetTimer(TH_CooldownTimer,this,&ABoostPad::RemoveActorCD,BoostPadCooldown);	
}

void ABoostPad::RemoveActorCD() 
{
	if(CooldownList.Num() > 0)
	{
		CooldownList.Pop();
	}

	ClientResetPad();
}

void ABoostPad::ClientResetPad() 
{
	bIsPrimed = false;
	PadMesh->SetMaterial(0,DefaultMaterial);
}

bool ABoostPad::WithinConeRange(FVector& PadLocation, FVector& HitActorLocation) 
{
	// Calculate cone range
	FVector FrontFacingVector = PadLocation + GetActorRotation().RotateVector(FVector(0,100,0));

	FVector ForwardFacingDirection = FrontFacingVector - PadLocation;
	ForwardFacingDirection.Normalize();

	FVector TargetFacingDirection = HitActorLocation - PadLocation;
	TargetFacingDirection.Normalize();

	// Check if Actor is within cone
	float AngleToFront = FMath::RadiansToDegrees(acosf(FVector::DotProduct(ForwardFacingDirection, TargetFacingDirection)));

	if(AngleToFront > 80.f) {return false;}
	else {return true;}
}

FVector ABoostPad::GetImpulseDirection(FVector& ActorLocation) 
{
	// Calculate Impulse direction+amount
	FVector ImpulseDirection = (ActorLocation + HeightOffset ) - GetActorLocation();
	ImpulseDirection.Normalize();
	ImpulseDirection*= BoostAmount;
		
	return ImpulseDirection;
}

bool ABoostPad::ClientPrimePad(AWokeAndShootCharacter* Initiator) 
{
	if(!CooldownList.Contains(Initiator) && !bIsPrimed)
	{	
		bIsPrimed = true;
		PadMesh->SetMaterial(0,PrimedMaterial);
		GetWorld()->GetTimerManager().SetTimer(TH_PadTimeOutTimer,this,&ABoostPad::ClientResetPad,BoostPadCooldown);
		return true;	
	}

	return false;
}

void ABoostPad::DetonatePad(AWokeAndShootCharacter* Initiator) 
{
	if(CooldownList.Contains(Initiator) || !bIsPrimed){return;}

	// Add Cooldown
	InitiateCooldown(Initiator);

	if(Initiator->IsLocallyControlled())
	{
		PadMesh->SetMaterial(0,DetonatedMaterial);
		GetWorld()->GetTimerManager().ClearTimer(TH_PadTimeOutTimer);
	}

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
			AWokeAndShootCharacter* HitActor = Cast<AWokeAndShootCharacter>(HitResult.GetActor());
			if (HitActor != nullptr)
			{
				FVector ActorLocation = HitActor->GetActorLocation();

				if(!WithinConeRange(SweepStart, ActorLocation)){continue;}

				FVector ImpulseDirection = GetImpulseDirection(ActorLocation);

				if(HitActor != Initiator)
				{
					continue;
				}

				// Apply Impulse
				HitActor->DirectionalImpulse(ImpulseDirection);
				

				if(Debug_DetonatePad)
				{
					// Debug Line for Forward Facing Line
					DrawDebugLine(GetWorld(),SweepStart ,(SweepStart + GetActorRotation().RotateVector(FVector(0,100,0))), FColor::Red, true);
				}
			}
		}
	}
	if(Debug_DetonatePad)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), MyColSphere.GetSphereRadius(), 50, FColor::Purple, true);
		GLog->Log("Boost Activated");
	}
}


