// Fill out your copyright notice in the Description page of Project Settings.



#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
#include "../Character/WokeAndShootCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Camera/CameraComponent.h"


// PhysFalling Override
void UMyCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations) 
{
    Super::PhysFalling(deltaTime, Iterations);
	
	if(TH_AirStrafeReset.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(TH_AirStrafeReset);
	}

	if(Velocity.Size() < 1000.f){return;}

	// Air Strafe Implementation
	AWokeAndShootCharacter* MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner());
	if(MyCharacter == nullptr)
	{
		return;
	}

	if(Velocity.Size2D() < 1200.f)
	{
		MaxWalkSpeed = 1200.f;
	}
	
	// Player Keyboard Inputs
	float MoveRightAxis = MyCharacter->Client_MoveRightAxis;
	float MoveForwardAxis = MyCharacter->Client_MoveForwardAxis;
	
	if(MoveRightAxis)
	{
		BrakingFrictionFactor = 0.f;

		FVector NormalVelocity = Velocity.GetSafeNormal2D();
		FVector ForwardVector = MyCharacter->GetActorForwardVector();
		FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
		FVector WishDir = InputAxis.RotateAngleAxis(MyCharacter->GetViewRotation().Yaw,FVector (0,0,1));
		
		if(MaxWalkSpeed < 1600.f)
		{
			MaxWalkSpeed += 10.f ;
		}
		else if (MaxWalkSpeed < MaxAirSpeed)
		{
			MaxWalkSpeed += 3.f ;
		}

		float CurrentSpeed = FVector::DotProduct(Velocity,WishDir);
		float MaxAccelDeltaTime = MaxAcceleration * deltaTime;
		float AddSpeed = MaxWalkSpeed - CurrentSpeed;

		AddSpeed = FMath::Max(FMath::Min(MaxAccelDeltaTime, AddSpeed),0.f);

		float TargetXVel = WishDir.X * abs(Velocity.X) * AddSpeed * StrafeMultiplier;
		float TargetYVel = WishDir.Y * abs(Velocity.Y) * AddSpeed * StrafeMultiplier;

		FVector NewVelocity = Velocity  + (WishDir * AddSpeed * StrafeMultiplier);

		float AimAtAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(NormalVelocity, ForwardVector)));
		float PositiveOrNegative = FVector::DotProduct(FVector::CrossProduct(NormalVelocity, ForwardVector),FVector(0,0,1));

		AimAtAngle *= PositiveOrNegative;


		if(AimAtAngle < 90 && (MoveRightAxis*AimAtAngle > 0	))
		{
			bIgnoreClientMovementErrorChecksAndCorrection = true;

			// 10.f and 30 are magic numbers and control the feel of the airstrafe, change them if future tuning is needed.
			float ModifiedAimAtAngle = (AimAtAngle - ((FMath::Clamp(WishDir.Size2D() * StrafeMultiplier, 0.f, 1.f))) * PositiveOrNegative);
			float AmountToRotate = FMath::FInterpTo(ModifiedAimAtAngle, AimAtAngle, deltaTime, 10);
			
			Velocity = NewVelocity.RotateAngleAxis(AmountToRotate, FVector (0,0,1));
			// Velocity = NewVelocity;

			const FVector Adjusted = Velocity * deltaTime * 0.;
			FHitResult Hit(1.f);
			FRotator NoPitchRotation = MyCharacter->GetViewRotation();
			NoPitchRotation.Pitch = 0;
			SafeMoveUpdatedComponent(Adjusted.GetSafeNormal(), NoPitchRotation, true, Hit);
		}

		// Debug mode
		if(Debug_AirStrafing)
		{
			UE_LOG(LogTemp,Warning,TEXT("________AIRSTRAFE LOG________"));
			UE_LOG(LogTemp,Warning,TEXT("VELOCITY: %s"),*Velocity.ToString());
			UE_LOG(LogTemp,Warning,TEXT("WISHDIR: %s"),*WishDir.ToString());
			UE_LOG(LogTemp,Warning,TEXT("SPEED: %f"),Velocity.Size2D());
			UE_LOG(LogTemp,Warning,TEXT("AimAtAngle: %f"),AimAtAngle);
			UE_LOG(LogTemp,Warning,TEXT("MoveRightAxis: %f"),MoveRightAxis);		
		}
	}

	
}

void UMyCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations) 
{
	Super::PhysWalking(deltaTime, Iterations);
	if(MaxWalkSpeed>1200.f && !TH_AirStrafeReset.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(TH_AirStrafeReset,this,&UMyCharacterMovementComponent::ResetAirMaxSpeed,AirStrafeGracePeriod);	
	}
}

void UMyCharacterMovementComponent::ResetAirMaxSpeed() 
{
	MaxWalkSpeed = 1200.f;
	BrakingFrictionFactor = 2.f;
}

void UMyCharacterMovementComponent::DynamicFOV() 
{
	float Speed = Velocity.Size2D();

	if(Speed > 1300.f && DynamicFOVSwitch)
	{	
		if(auto MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner()))
		{
			UCameraComponent* FPC = MyCharacter->GetFirstPersonCameraComponent();
			float BaseFOV = MyCharacter->BaseFOV;
			// float AddFOV = (DynamicFOVAmount / (MaxAirSpeed - 1300.f)) * (Speed - 1300.f);
			float AddFOV = BaseFOV + (DynamicFOVAmount / (MaxAirSpeed - 1300.f)) * (Speed - 1300.f);
			FPC->FieldOfView = AddFOV;
			
			if(FPC->FieldOfView > BaseFOV + DynamicFOVAmount+1.f)
			{
				DynamicFOVSwitch = false;
			}
		}
	} 
	else if(!DynamicFOVSwitch && Speed < 1300.f)
	{
		if(auto World = GetWorld())
		{
			if(auto MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner()))
			{
				UCameraComponent* FPC = MyCharacter->GetFirstPersonCameraComponent();
				FPC->FieldOfView = FMath::FInterpTo(FPC->FieldOfView,MyCharacter->BaseFOV,World->GetDeltaSeconds(),10);

				if(FPC->FieldOfView < MyCharacter->BaseFOV+1.f)
				{
					DynamicFOVSwitch = true;
				}
			}
		}
		
	}

}

void UMyCharacterMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) 
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	DynamicFOV();
}
