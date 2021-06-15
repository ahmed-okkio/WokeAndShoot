// Fill out your copyright notice in the Description page of Project Settings.



#include "MyCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/InputSettings.h"
#include "WokeAndShootCharacter.h"
#include "Kismet/KismetMathLibrary.h"
// #include "GameFramework/PlayerController.h"
// #include "Engine/World.h"


//PhysFalling Override
void UMyCharacterMovementComponent::PhysFalling(float deltaTime, int32 Iterations) 
{
    Super::PhysFalling(deltaTime, Iterations);
	// GLog->Log("Custom Falling Active");

	//Air Strafe Implementation
	AWokeAndShootCharacter* MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner());
	if(MyCharacter == nullptr)
		return;

	float MoveRightAxis = MyCharacter->Client_MoveRightAxis;
	float MoveForwardAxis = MyCharacter->Client_MoveForwardAxis;

	

	UE_LOG(LogTemp,Warning,TEXT("ForwardAxis: %f"),MoveForwardAxis);
	UE_LOG(LogTemp,Warning,TEXT("RightAxis: %f"),MoveRightAxis);

	FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
	FVector WishDir = InputAxis.RotateAngleAxis(GetPawnOwner()->GetViewRotation().Yaw,FVector (0,0,1));

	float CurrentSpeed = FVector::DotProduct(Velocity,WishDir);
	float MaxAccelDeltaTime = MaxAcceleration * deltaTime;
	float AddSpeed = MaxWalkSpeed - CurrentSpeed;

	AddSpeed = FMath::Max(FMath::Min(MaxAccelDeltaTime, AddSpeed),0.f);

	
	WishDir.Normalize();
	float TargetXVel = WishDir.X * Velocity.Size() * AddSpeed * StrafeMultiplier;
	float TargetYVel = WishDir.Y * Velocity.Size() * AddSpeed * StrafeMultiplier;
	
	

	Velocity.X = FMath::Lerp(Velocity.X, TargetXVel,StrafeSharpness);
	Velocity.Y = FMath::Lerp(Velocity.Y, TargetYVel,StrafeSharpness);



	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	// UE_LOG(LogTemp,Warning,TEXT("%f"),Velocity.Size());
	// UE_LOG(LogTemp,Warning,TEXT("%s"),*FinalImpulse.ToString());
	// UE_LOG(LogTemp,Warning,TEXT("________NEW LOG________"));
	// UE_LOG(LogTemp,Warning,TEXT("%s"),*Velocity.ToString());
	// if(GetOwner()->HasAuthority())
	// {
	// 	// GLog->Log("RUNNING ON SERVER");
	// 	// UE_LOG(LogTemp,Warning,TEXT("%f"),MoveRightAxis);
	// 	// UE_LOG(LogTemp,Warning,TEXT("%f"),MoveForwardAxis);
	// }
	// if(!GetOwner()->HasAuthority())
	// {
	// 	// GLog->Log("RUNNING ON CLIENT");
	// }	
}
