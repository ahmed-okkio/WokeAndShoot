// Fill out your copyright notice in the Description page of Project Settings.


#include "BoostPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "WokeAndShoot/WokeAndShootProjectile.h"
#include "WokeAndShoot/WokeAndShootCharacter.h"
#include "DrawDebugHelpers.h"
// Sets default values
ABoostPad::ABoostPad()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("BoostPad");

	RootComponent = CollisionComp;
	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoostPadMesh"));
	PadMesh->AttachTo(CollisionComp);
	if(HasAuthority())
	{
		PadMesh->OnComponentHit.AddDynamic(this, &ABoostPad::OnHit);
	}
}

// Called when the game starts or when spawned
void ABoostPad::BeginPlay()
{
	Super::BeginPlay();
}

void ABoostPad::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) 
{
	//Allow only bullets to activate
	if(!OtherActor->IsA(AWokeAndShootProjectile::StaticClass()))
		return;
	
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

				if (CooldownList.Contains(HitActor)){continue;}
				if(!WithinConeRange(SweepStart, ActorLocation)){continue;}

				//Calculate Impulse direction+amount
				FVector ImpulseDirection = (ActorLocation + HeightOffset ) - GetActorLocation();
				ImpulseDirection.Normalize();
				ImpulseDirection*= BoostAmount;

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

// Called every frame
void ABoostPad::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABoostPad::InitiateCooldown(AWokeAndShootCharacter* HitActor) 
{
	CooldownList.AddUnique(HitActor);
	FTimerHandle TH_CooldownTimer;
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

	if(AngleToFront > 80.f)
		return false;
	else
		return true;
}

// bool ABoostPad::Server_RelayBoost_Validate(FVector SweepStart, FVector ActorLocation, FVector ImpulseDirection, AWokeAndShootCharacter* HitActor ) 
// {
// 	return true;
// }

// void ABoostPad::Server_RelayBoost_Implementation(FVector SweepStart, FVector ActorLocation, FVector ImpulseDirection, AWokeAndShootCharacter* HitActor ) 
// {
// 	if (CooldownList.Contains(HitActor)){return;}
// 	if(!WithinConeRange(SweepStart, ActorLocation)){return;}
// 	GLog->Log("Passed range and cd check");
// 	//Apply Impulse
// 	HitActor->DirectionalImpulse(ImpulseDirection);
// 	//Add Cooldown
// 	InitiateCooldown(HitActor);
// 	Multi_RelayBoost(ActorLocation, HitActor);

// }

// bool ABoostPad::Multi_RelayBoost_Validate(FVector ActorLocation, AWokeAndShootCharacter* HitActor) 
// {
// 	return true;
// }

// void ABoostPad::Multi_RelayBoost_Implementation(FVector ActorLocation, AWokeAndShootCharacter* HitActor) 
// {
// 	//Calculate Impulse direction+amount
// 	FVector ImpulseDirection = (ActorLocation + HeightOffset ) - GetActorLocation();
// 	ImpulseDirection.Normalize();
// 	ImpulseDirection*= BoostAmount;

// 	//Apply Impulse
// 	HitActor->DirectionalImpulse(ImpulseDirection);
// }
