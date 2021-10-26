// Fill out your copyright notice in the Description page of Project Settings.



#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
#include "../Character/WokeAndShootCharacter.h"
#include "Kismet/KismetMathLibrary.h"


// PhysFalling Override
void UMyCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations) 
{
    Super::PhysFalling(deltaTime, Iterations);
	// GravityScale = 1.8f * (0.9 + ((1200 - Velocity.Size2D())/12000));
	// UE_LOG(LogTemp,Warning,TEXT("Gravity: %f"),GravityScale);	
	// UE_LOG(LogTemp,Warning,TEXT("Vel: %f"),((1200 - Velocity.Size2D())/12000));	

	if(Velocity.Size2D() < 500.f && GravityScale < 1.8f)
	{
		// velocity = 100% : gravity scale = 0.9%
		// velocity = 80% : gravity = 0.92%
		// Gravity = 1.8 (100%) Velocity = 500(50%);
		// 
		// * (Velocity.Size2D()/1200.f - 1.f);
	}
	if(Velocity.Size() < 1000.f){return;}
	// Air Strafe Implementation
	AWokeAndShootCharacter* MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner());
	if(MyCharacter == nullptr)
	{
		return;
	}

	// Player Keyboard Inputs
	float MoveRightAxis = MyCharacter->Client_MoveRightAxis;
	float MoveForwardAxis = MyCharacter->Client_MoveForwardAxis;
	
	if(MoveRightAxis != 0)
	{
		FVector NormalVelocity = Velocity.GetSafeNormal2D();
		FVector ForwardVector = MyCharacter->GetActorForwardVector();
		FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
		FVector WishDir = InputAxis.RotateAngleAxis(MyCharacter->GetViewRotation().Yaw,FVector (0,0,1));

		float CurrentSpeed = FVector::DotProduct(Velocity,WishDir);
		float MaxAccelDeltaTime = MaxAcceleration * deltaTime;
		float AddSpeed = MaxWalkSpeed - CurrentSpeed;

		AddSpeed = FMath::Max(FMath::Min(MaxAccelDeltaTime, AddSpeed),0.f);

		float TargetXVel = WishDir.X * abs(Velocity.X) * AddSpeed * StrafeMultiplier;
		float TargetYVel = WishDir.Y * abs(Velocity.Y) * AddSpeed * StrafeMultiplier;

		FVector NewVelocity = Velocity  + (WishDir * AddSpeed * StrafeMultiplier);

		float AimAtAngle = FMath::RadiansToDegrees(FVector::DotProduct(NormalVelocity, ForwardVector));
		float PositiveOrNegative = FVector::DotProduct(FVector::CrossProduct(NormalVelocity, ForwardVector),FVector(0,0,1));
		AimAtAngle *= PositiveOrNegative;

		// Applying Strafe Sharpness Modifier
		AimAtAngle *= StrafeSharpness*WishDir.Size2D();;

		if(AimAtAngle)
		{
			bIgnoreClientMovementErrorChecksAndCorrection = true;
			Velocity = NewVelocity.RotateAngleAxis(AimAtAngle,FVector (0,0,1));
			// Velocity = NewVelocity;
		}

		// Debug mode
		if(Debug_AirStrafing)
		{
			UE_LOG(LogTemp,Warning,TEXT("________AIRSTRAFE LOG________"));
			UE_LOG(LogTemp,Warning,TEXT("VELOCITY: %s"),*Velocity.ToString());
			UE_LOG(LogTemp,Warning,TEXT("WISHDIR: %s"),*WishDir.ToString());
			UE_LOG(LogTemp,Warning,TEXT("SPEED: %f"),Velocity.Size2D());	
			UE_LOG(LogTemp,Warning,TEXT("________AIRSTRAFE LOG________"));
		}
	}

	const FVector Adjusted = Velocity * deltaTime * 0.5;
	FHitResult Hit(1.f);
	FRotator NoPitchRotation = MyCharacter->GetViewRotation();
	NoPitchRotation.Pitch = 0;
	SafeMoveUpdatedComponent(Adjusted, NoPitchRotation, true, Hit);

}
