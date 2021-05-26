// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootCharacter.h"
#include "WokeAndShootProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
// #include "Components/InputComponent.h"
#include "GameFramework/InputSettings.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "WokeAndShoot/WokeAndShootProjectile.h"
#include "GameFramework/CharacterMovementComponent.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);
#define FLAG(x) UE_LOG(LogTemp,Warning, TEXT(x))
//////////////////////////////////////////////////////////////////////////
// AWokeAndShootCharacter
AWokeAndShootCharacter::AWokeAndShootCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	Mesh1P->SetRelativeRotation(FRotator(1.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-0.5f, -4.4f, -155.7f));

	// Create a gun mesh component
	FP_Gun = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FP_Gun"));
	FP_Gun->SetOnlyOwnerSee(false);			// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	// FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun,TEXT("Muzzle"));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	//Set Character Movement Component
	CharacterMovement = GetCharacterMovement();
	// Default offset from the character location for projectiles to spawn
	// GunOffset = FVector(100.0f, 0.0f, 10.0f);
}

void AWokeAndShootCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), TEXT("GripPoint"));
}

void AWokeAndShootCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
	
	// UE_LOG(LogTemp,Warning,TEXT("FV: %s"),*GetVelocity().ToString());
	AirStrafeHandler(DeltaTime)	;
}

// Input
void AWokeAndShootCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Instead of calling Super
	check(PlayerInputComponent);

	// Bind jump events
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAxis("Wheel", this, &AWokeAndShootCharacter::JumpHandler);

	// Bind fire event
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AWokeAndShootCharacter::OnFire);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AWokeAndShootCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWokeAndShootCharacter::MoveRight);

	// Bind turn events
	PlayerInputComponent->BindAxis("Turn", this, &AWokeAndShootCharacter::LookLeftRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AWokeAndShootCharacter::LookUpDown);
}



void AWokeAndShootCharacter::OnFire()
{
	// Prepare parameters for line trace and hit scan
	FVector ViewPointLocation; 
	FRotator ViewPointRotation;
	GetViewPointRotLoc(ViewPointLocation,ViewPointRotation);

	FVector EndPoint = ViewPointLocation + ViewPointRotation.Vector() * Range;

	// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
	if (FP_MuzzleLocation == nullptr)
		return;

	FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();
	FVector ShotDirection = FP_MuzzleLocation->GetComponentRotation().Vector();
	FRotator SpawnRotation =  FRotationMatrix::MakeFromX(EndPoint - SpawnLocation).Rotator();
	
	UWorld* const World = GetWorld();
	if (World != nullptr)
	{
		//LineTrace
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);
		bool bLineTrace = World->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndPoint,ECollisionChannel::ECC_GameTraceChannel2, Params);
		
		if (bLineTrace)
		{
			float Distance = FVector::Dist(SpawnLocation, HitResult.Location);

			AActor* Projectile = World->SpawnActor<AWokeAndShootProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			Projectile->SetOwner(this);
			ProjectileSceneComp = Projectile->FindComponentByClass<USceneComponent>();

			//Bullet Impact + Tracer
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitResult.Location, ShotDirection.Rotation());
			DrawBulletTracers(ViewPointLocation, SpawnLocation, HitResult.Location, ShotDirection, Distance);
		}
		else 
		{
			DrawBulletTracers(ViewPointLocation, SpawnLocation, EndPoint, ShotDirection, Range);
		}
	} 	

	// try and play the sound if specified
	if (FireSound != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
	}

	// try and play a firing animation if specified
	if (FireAnimation != nullptr)
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance();
		if (AnimInstance != nullptr)
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
	// DrawDebugLine(World,ViewPointLocation,HitResult.Location, FColor::Red, true);
	// UE_LOG(LogTemp,Warning, TEXT("Result is: %s"),bLineTrace? TEXT("True") : TEXT("False"));
}

//Potentially delete
void AWokeAndShootCharacter::UpdateTracerSource(UParticleSystemComponent* TracerComponent, FVector DynamicTracerSource, FVector Target) 
{
	// DynamicSourceTest = FMath::VInterpConstantTo(DynamicSourceTest,Target,GetWorld()->GetDeltaSeconds(),10000);
	// TracerComponent->SetBeamSourcePoint(0, DynamicSourceTest, 0);
	// UE_LOG(LogTemp,Warning, TEXT("Timer Function Set Source + Lerp2 %s "), *(DynamicSourceTest-Target).ToString());
	// if (DynamicSourceTest == Target)
	// {
	// 	FLAG("EQUAL");
	// }
}


void AWokeAndShootCharacter::GetViewPointRotLoc(FVector &ViewPointLocation, FRotator &ViewPointRotation) const
{
	AController* OwnerController = GetController();
	if (OwnerController == nullptr) return;
	
	OwnerController->GetPlayerViewPoint(ViewPointLocation, ViewPointRotation);
}



void AWokeAndShootCharacter::DrawBulletTracers(FVector &ViewPointLocation, FVector &SpawnLocation, FVector &HitResultLocation, FVector &ShotDirection, float Distance)
{
	//Refactor when tracers are complete
	UParticleSystemComponent* TracerComponent;
	if (Distance == Range)
	{
		FLAG("NEW EMITTER");
		UE_LOG(LogTemp, Warning , TEXT("%s"),*HitResultLocation.ToString());		
		TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),TracerParticle,SpawnLocation, ShotDirection.Rotation());
	}
	else
	{ 		
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(TracerParticle, ProjectileSceneComp);
	}

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	if (TracerParticle)
	{
		FVector DynamicTracerSource = SpawnLocation;
		FVector Target =  HitResultLocation;
		float OutDistance;
		// FTimerHandle TracerUpdateTH;
		// FTimerDelegate UpdateTracerSourceDelegate = FTimerDelegate::CreateUObject(this,&AWokeAndShootCharacter::UpdateTracerSource, TracerComponent, DynamicTracerSource, Target);
		TracerComponent->SetFloatParameter(TEXT("Distance"), Distance-500);
		TracerComponent->GetFloatParameter("Distance", OutDistance);
		UE_LOG(LogTemp,Warning, TEXT("%f"),Distance);
		TracerComponent->SetBeamSourcePoint(0, DynamicTracerSource, 0);
		TracerComponent->SetBeamTargetPoint(0, HitResultLocation, 0);
		FLAG("TEXT");
		if (TracerComponent->IsActive() == false)
		{
			UE_LOG(LogTemp,Warning, TEXT("TEST2"));
			TracerComponent->SetActive(true, true);
			UE_LOG(LogTemp,Warning, TEXT("TRACER SET TO ACTIVE, Distance: %f"), Distance);
		}
		DynamicSourceTest = DynamicTracerSource;
		// GetWorld()->GetTimerManager().SetTimer(TracerUpdateTH, UpdateTracerSourceDelegate, 0.08,true);
	}
	else if (TracerComponent->IsActive() == true)
	{
		UE_LOG(LogTemp,Warning, TEXT("TEST3"));
		TracerComponent->SetActive(false, true);
	}
}



//Movement
void AWokeAndShootCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		//Wishdir (airstrafe) params, Wish dir is only calculated while pressing W
		float MoveRightAxis = InputComponent->GetAxisValue(TEXT("MoveRight"));
		float MoveForwardAxis = Value;
		FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
		WishDir = InputAxis.RotateAngleAxis(FirstPersonCameraComponent->GetComponentRotation().Yaw,FVector (0,0,1));

		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value);
	}
}

void AWokeAndShootCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		//Restricts left right speed when in air
		if(CharacterMovement->MovementMode.GetValue() == MOVE_Falling)
			Value *= 0.3;
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value);
	}
}

void AWokeAndShootCharacter::JumpHandler(float Val) 
{
	if(Val == 1.f)
	{
		ACharacter::Jump();
	}
}

void AWokeAndShootCharacter::LookUpDown(float Rate)
{
	APawn::AddControllerPitchInput(Sensitivity * Rate);
}

void AWokeAndShootCharacter::LookLeftRight(float Rate)
{
	APawn::AddControllerYawInput(Sensitivity * Rate);
}

void AWokeAndShootCharacter::DirectionalImpulse(FVector ImpulseDirection) 
{
	CharacterMovement->AddImpulse(ImpulseDirection);
}

void AWokeAndShootCharacter::AirStrafeHandler(float& DeltaTime) 
{
	if(CharacterMovement->MovementMode.GetValue() == MOVE_Falling)
	{
		float CurrentSpeed = FVector::DotProduct(GetVelocity(),WishDir);

		float MaxAccelDeltaTime = CharacterMovement->MaxAcceleration * DeltaTime;

		float AddSpeed = CharacterMovement->MaxWalkSpeed - CurrentSpeed;
		AddSpeed = FMath::Min(MaxAccelDeltaTime, AddSpeed);
		AddSpeed = FMath::Max(AddSpeed,0.f);

		FVector FinalImpulse = (WishDir * AddSpeed *0.8);
		
		CharacterMovement->AddImpulse(FinalImpulse,false);
		
	}
	else
	{
		WishDir = FVector (0,0,0);
	}
}
