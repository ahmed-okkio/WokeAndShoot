// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SceneComponent.h"
#include "MyCharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CharacterComponenets/HealthComponent.h"
#include "BoostPad.h"
// #include "DrawDebugHelpers.h"

//Potentially depricated
// #include "GameFramework/CharacterMovementComponent.h"
// #include "Particles/ParticleSystem.h"
// #include "HeadMountedDisplayFunctionLibrary.h"
// #include "WokeAndShootProjectile.h"
// #include "GameFramework/InputSettings.h"


DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);
#define FLAG(x) UE_LOG(LogTemp,Warning, TEXT(x))
//////////////////////////////////////////////////////////////////////////
// AWokeAndShootCharacter
AWokeAndShootCharacter::AWokeAndShootCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	CreateCameraComp();

	//Create mesh components
	CreateMeshComps();

	//SetMuzzleLocation
	SetMuzzleLocation();

	//Set Character Movement Component
	CharacterMovement = GetCharacterMovement();

	//Set Health Component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AWokeAndShootCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), TEXT("GripPoint"));
}

void AWokeAndShootCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
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

void AWokeAndShootCharacter::CustomTakeDamage(float DamageAmount) 
{
	HealthComponent->ApplyDamage(DamageAmount);
}

void AWokeAndShootCharacter::CustomTakeHeal(float HealAmount) 
{
	HealthComponent->ApplyHeal(HealAmount);	
}


//Refactor 
void AWokeAndShootCharacter::OnFire()
{
	// Prepare parameters for line trace and hit scan
	FVector ViewPointLocation; 
	FRotator ViewPointRotation;
	GetViewPointRotLoc(ViewPointLocation,ViewPointRotation);

	FVector EndPoint = ViewPointLocation + ViewPointRotation.Vector() * Range;

	if (FP_MuzzleLocation == nullptr){return;}	

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
		if(HasAuthority())
		{
			bool bLineTrace = World->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndPoint,ECollisionChannel::ECC_GameTraceChannel2, Params);
			if (bLineTrace)
			{
				//Kill Player Hit
				// HandleDeath();
				AWokeAndShootCharacter* Character = Cast<AWokeAndShootCharacter>(HitResult.GetActor());
				if(Character)
				{
					Character->HealthComponent->ApplyDamage(100.f);
					Multi_RelayDamage(100.f, HitResult.GetActor());
				}
			}
		}
		else
		{
			Server_RelayHitScan(World, ViewPointLocation, EndPoint);
		}

		bool bOfflineLineTrace = World->LineTraceSingleByChannel(HitResult, ViewPointLocation, EndPoint,ECollisionChannel::ECC_GameTraceChannel2, Params);
		
		if (bOfflineLineTrace)
		{
			float Distance = FVector::Dist(SpawnLocation, HitResult.Location);
			// AActor* Projectile = World->SpawnActor<AWokeAndShootProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
			// Projectile->SetOwner(this);
			// ProjectileSceneComp = Projectile->FindComponentByClass<USceneComponent>();

			//Check if ActorHit is a BoostPad
			
			ABoostPad* HitBoostPad = Cast<ABoostPad>(HitResult.GetActor());
			if(HitBoostPad)
			{
				
				HitBoostPad->BoostPlayers(this);

				//Network
				if(!HasAuthority())
				{
					Server_RelayBoost(HitBoostPad);
				}
			}


			//Bullet Impact + Tracer
			if(BulletImpact)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitResult.Location, ShotDirection.Rotation());
			}


			// DrawBulletTracers(ViewPointLocation, SpawnLocation, HitResult.Location, ShotDirection, Distance);

			// START HERE
			//Network
			if(!HasAuthority())
			{
				Server_RelayShot(HitResult, SpawnLocation, SpawnRotation, HitResult.Location, ShotDirection.Rotation());
			}
			else
			{
				Multi_RelayShot(HitResult, SpawnLocation, SpawnRotation, HitResult.Location, ShotDirection.Rotation());
			}
		}
		else 
		{
			// DrawBulletTracers(ViewPointLocation, SpawnLocation, EndPoint, ShotDirection, Range);
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
	if(TracerParticle == nullptr){return;}
	if (Distance == Range)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),TracerParticle,SpawnLocation, ShotDirection.Rotation());
	}
	else
	{ 		
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(TracerParticle, ProjectileSceneComp);
	}

	//Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	if (TracerComponent)
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
		TracerComponent->SetActive(false, true);
	}
}



//Movement
void AWokeAndShootCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// //Wishdir (airstrafe) params, Wish dir is only calculated while pressing W
		// float MoveRightAxis = InputComponent->GetAxisValue(TEXT("MoveRight"));
		// float MoveForwardAxis = Value;
		// FVector InputAxis = FVector(MoveForwardAxis,MoveRightAxis,0);
		// WishDir = InputAxis.RotateAngleAxis(FirstPersonCameraComponent->GetComponentRotation().Yaw,FVector (0,0,1));
		AddMovementInput(GetActorForwardVector(), Value);
	}
	else if(CharacterMovement->MovementMode.GetValue() == MOVE_Falling)
	{
		//Keep Movement going while air strafing without holding W
		FVector VelocityNoZ = UKismetMathLibrary::LessLess_VectorRotator(GetVelocity(),GetActorRotation());
		if(VelocityNoZ.X > 600)
			Value = 1.f;
		else if(VelocityNoZ.X < -600)
			Value = -1.f;
	}
	
	AddMovementInput(GetActorForwardVector(), Value);
	Client_MoveForwardAxis = Value;
	//Relay Axis to Server
	Server_RelayForwardAxis(Value);
	
}

void AWokeAndShootCharacter::MoveRight(float Value)
{
	//Set Local Axis Variable
	Client_MoveRightAxis = Value;
	// Relay Axis to Server
	Server_RelayRightAxis(Value);
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

void AWokeAndShootCharacter::HandleDeath() 
{
	CharacterMovement->DisableMovement();
}

void AWokeAndShootCharacter::LookUpDown(float Rate)
{
	APawn::AddControllerPitchInput(Sensitivity * Rate);

	//Network
	if(HasAuthority())
	{
		Multi_RelayPitch(GetViewRotation().Pitch);
	}
	else
	{
		Server_RelayPitch(GetViewRotation().Pitch);
	}
	
}

void AWokeAndShootCharacter::LookLeftRight(float Rate)
{
	APawn::AddControllerYawInput(Sensitivity * Rate);
}

void AWokeAndShootCharacter::DirectionalImpulse(FVector ImpulseDirection) 
{

	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;
	CharacterMovement->Launch(ImpulseDirection);
	// if(!HasAuthority())
	// {
	// 	Server_RelayBoost(ImpulseDirection);
	// 	GLog->Log("Applying Launch On Server");
	//
	// }
	// else
	// {
	// 	// GLog->Log("Client Launched By Server");
	// 	Multi_RelayBoost(ImpulseDirection);
	// }

}

void AWokeAndShootCharacter::Landed(const FHitResult & Hit) 
{
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = false;
}

bool AWokeAndShootCharacter::IsDead() const
{
	if(HealthComponent->HealthPoints == 0)
	{
		return true;
	}
	return false;
}

//Relay Pitch
bool AWokeAndShootCharacter::Server_RelayPitch_Validate(float Pitch) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayPitch_Implementation(float Pitch) 
{

	FRotator CameraRotation = FirstPersonCameraComponent->GetRelativeRotation();
	CameraRotation.Pitch = Pitch;
	FirstPersonCameraComponent->SetRelativeRotation(CameraRotation);
	
	Multi_RelayPitch(GetViewRotation().Pitch);
}

bool AWokeAndShootCharacter::Multi_RelayPitch_Validate(float Pitch) 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayPitch_Implementation(float Pitch) 
{
	if(!IsLocallyControlled())
	{
		FRotator CameraRotation = FirstPersonCameraComponent->GetRelativeRotation();
		CameraRotation.Pitch = Pitch;
		FirstPersonCameraComponent->SetRelativeRotation(CameraRotation);
	}
	
}


//Relay Shot
bool AWokeAndShootCharacter::Server_RelayShot_Validate(FHitResult HitResult, FVector SpawnLocation, FRotator SpawnRotation, FVector HitLocation, FRotator ShotDirection) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayShot_Implementation(FHitResult HitResult, FVector SpawnLocation, FRotator SpawnRotation, FVector HitLocation, FRotator ShotDirection) 
{
	// AActor* Projectile = GetWorld()->SpawnActor<AWokeAndShootProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
	// Projectile->SetOwner(this);
	// ProjectileSceneComp = Projectile->FindComponentByClass<USceneComponent>();
	//Bullet Impact + Tracer
	if(BulletImpact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitLocation, ShotDirection);
	}

	Multi_RelayShot(HitResult, SpawnLocation, SpawnRotation, HitLocation, ShotDirection);
}

bool AWokeAndShootCharacter::Multi_RelayShot_Validate(FHitResult HitResult, FVector SpawnLocation, FRotator SpawnRotation, FVector HitLocation, FRotator ShotDirection) 
{
	return true;	
}

void AWokeAndShootCharacter::Multi_RelayShot_Implementation(FHitResult HitResult, FVector SpawnLocation, FRotator SpawnRotation, FVector HitLocation, FRotator ShotDirection) 
{
	if(!IsLocallyControlled())
	{
		// AActor* Projectile = GetWorld()->SpawnActor<AWokeAndShootProjectile>(ProjectileClass, SpawnLocation, SpawnRotation);
		// Projectile->SetOwner(this);
		// ProjectileSceneComp = Projectile->FindComponentByClass<USceneComponent>();
		//Bullet Impact + Tracer
		if(BulletImpact)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitLocation, ShotDirection);
		}
	}
}


//Relay Movement Axis Inputs
bool AWokeAndShootCharacter::Server_RelayForwardAxis_Validate(float MoveForwardAxisParam) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayForwardAxis_Implementation(float MoveForwardAxisParam) 
{
	Client_MoveForwardAxis = MoveForwardAxisParam;
}

bool AWokeAndShootCharacter::Server_RelayRightAxis_Validate(float MoveRightAxisParam) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayRightAxis_Implementation(float MoveRightAxisParam) 
{
	Client_MoveRightAxis = MoveRightAxisParam;
}

bool AWokeAndShootCharacter::Server_RelayHitScan_Validate(UWorld* World, const FVector& ViewPointLocation, const FVector& EndPoint) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayHitScan_Implementation(UWorld* World, const FVector& ViewPointLocation, const FVector& EndPoint) 
{
	//LineTrace
	FHitResult ServerHitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	bool bLineTrace = World->LineTraceSingleByChannel(ServerHitResult, ViewPointLocation, EndPoint,ECollisionChannel::ECC_GameTraceChannel2, Params);
	if(bLineTrace && ServerHitResult.GetActor())
	{
		// HandleDeath();
		AWokeAndShootCharacter* Character = Cast<AWokeAndShootCharacter>(ServerHitResult.GetActor());
		if(Character)
		{
			Character->HealthComponent->ApplyDamage(100.f);
			//Kill Player Hit On all Clients
			Multi_RelayDamage(100.f, ServerHitResult.GetActor());
		}
		
	}
}

bool AWokeAndShootCharacter::Multi_RelayDamage_Validate(float Damage, AActor* HitActor) 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayDamage_Implementation(float Damage, AActor* HitActor) 
{
	// HandleDeath();
	AWokeAndShootCharacter* Character = Cast<AWokeAndShootCharacter>(HitActor);
	if(Character)
	{
		Character->HealthComponent->ApplyDamage(Damage);
	}
	
}



bool AWokeAndShootCharacter::Server_RelayBoost_Validate(ABoostPad* HitBoostPad) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayBoost_Implementation(ABoostPad* HitBoostPad)
{
	HitBoostPad->BoostPlayers(this);
	Multi_RelayBoost(HitBoostPad);
}

bool AWokeAndShootCharacter::Multi_RelayBoost_Validate(ABoostPad* HitBoostPad) 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayBoost_Implementation(ABoostPad* HitBoostPad) 
{
	if(!IsLocallyControlled())
	{
		HitBoostPad->BoostPlayers(this);
	}
}
void AWokeAndShootCharacter::CreateCameraComp() 
{
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 64.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

void AWokeAndShootCharacter::CreateMeshComps() 
{
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
	FP_Gun->SetOnlyOwnerSee(true);
				
	// otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	//Create a third person body mesh component
	TP_Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third_Person Body"));
	TP_Body->SetOwnerNoSee(true);
	// TP_Body->SetOnlyOwnerSee(false);
	TP_Body->SetupAttachment(RootComponent);	
	
}

void AWokeAndShootCharacter::SetMuzzleLocation() 
{
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun,TEXT("Muzzle"));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
}
