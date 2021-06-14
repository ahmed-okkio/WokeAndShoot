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
	// UE_LOG(LogTemp,Warning,TEXT("%f"),MaxWalkSpeed);
	// UE_LOG(LogTemp,Warning,TEXT("%f"),Velocity.Size());
	
	AWokeAndShootCharacter* MyCharacter = Cast<AWokeAndShootCharacter>(GetOwner());
	if(MyCharacter == nullptr)
		return;
	float MoveRightAxis = MyCharacter->Client_MoveRightAxis;
	float MoveForwardAxis = MyCharacter->Client_MoveForwardAxis;
	
	if(MoveRightAxis == 0)
		return;

	FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
	FVector WishDir = InputAxis.RotateAngleAxis(GetPawnOwner()->GetViewRotation().Yaw,FVector (0,0,1));

	float CurrentSpeed = FVector::DotProduct(Velocity,WishDir);
	float MaxAccelDeltaTime = MaxAcceleration * deltaTime;
	float AddSpeed = MaxWalkSpeed - CurrentSpeed;

	AddSpeed = FMath::Min(MaxAccelDeltaTime, AddSpeed);
	AddSpeed = FMath::Max(AddSpeed,0.f);

	FVector FinalImpulse = (WishDir * AddSpeed * StrafeMultiplier);
	// Velocity += FinalImpulse;
	Velocity.X = FMath::FInterpTo(Velocity.X,WishDir.GetSafeNormal().X* Velocity.Size(),deltaTime,3);
	Velocity.Y = FMath::FInterpTo(Velocity.Y,WishDir.GetSafeNormal().Y* Velocity.Size(),deltaTime,3);
	// float CurrSpeedX =  Velocity.X/MaxWalkSpeed;
	// float CurrSpeedY =  Velocity.Y/MaxWalkSpeed;

	UE_LOG(LogTemp,Warning,TEXT("%f"),CurrentSpeed);
	// UE_LOG(LogTemp,Warning,TEXT("WishDir: %s"),*WishDir.GetSafeNormal().ToString());
	// if(Velocity.Size() > MaxWalkSpeed + 600 || Velocity.Size() < 300)
	// 	return;
	// Velocity.X = (WishDir.GetSafeNormal()*MaxWalkSpeed).X;
	// Velocity.Y = (WishDir.GetSafeNormal()*MaxWalkSpeed).Y;
	float AimAtAngle = FMath::RadiansToDegrees(acosf(FVector::DotProduct(Velocity.GetSafeNormal(), MyCharacter->GetActorForwardVector())));
	// Velocity = Velocity.RotateAngleAxis(AimAtAngle,FVector (0,0,1));

	
	

	// UE_LOG(LogTemp,Warning,TEXT("________NEW LOG________"));
	UE_LOG(LogTemp,Warning,TEXT("%s"),*(MyCharacter->GetViewRotation().Vector()*MaxWalkSpeed).ToString());
	UE_LOG(LogTemp,Warning,TEXT("%f"),AimAtAngle);
	UE_LOG(LogTemp,Warning,TEXT("%s"),*Velocity.ToString());
	UE_LOG(LogTemp,Warning,TEXT("ROTATED: %s"),*Velocity.RotateAngleAxis(AimAtAngle,FVector (0,0,1)).ToString());
	
	// FVector ForwardVector = MyCharacter->GetActorLocation() + MyCharacter->GetActorForwardVector();
	// FRotator ForwardVectorRotation = UKismetMathLibrary::FindLookAtRotation(MyCharacter->GetActorLocation(),ForwardVector);
	// Velocity = ForwardVectorRotation.RotateVector(Velocity);
	// Velocity *= MyCharacter->GetViewRotation().Vector();
	// MyCharacter->GetViewRotation()

	
	if(GetOwner()->HasAuthority())
	{
		// GLog->Log("RUNNING ON SERVER");
		// UE_LOG(LogTemp,Warning,TEXT("%f"),MoveRightAxis);
		// UE_LOG(LogTemp,Warning,TEXT("%f"),MoveForwardAxis);

	}
	if(!GetOwner()->HasAuthority())
	{
		
		// GLog->Log("RUNNING ON CLIENT");
	}
	const FVector Adjusted = Velocity * deltaTime;
	FHitResult Hit(1.f);
	SafeMoveUpdatedComponent(Adjusted, UpdatedComponent->GetComponentQuat(), true, Hit);

	// UE_LOG(LogTemp,Warning,TEXT("%f"),Velocity.Size());
	// UE_LOG(LogTemp,Warning,TEXT("%s"),*FinalImpulse.ToString());	
}
