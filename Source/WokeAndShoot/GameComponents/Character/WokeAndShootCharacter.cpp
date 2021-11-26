// Copyright Epic Games, Inc. All Rights Reserved.

#include "../CustomMovement/BoostPad.h"
#include "../CustomMovement/MyCharacterMovementComponent.h"
#include "../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "../PlayerController/WokeAndShootPlayerController.h"
#include "../../ServerComponents/PlayerState/MyPlayerState.h"
#include "../GameInstance/WnSGameInstance.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "CharacterComponenets/HealthComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Particles/ParticleSystemComponent.h"
#include "WokeAndShootCharacter.h"



DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);


//////////////////////////////////////////////////////////////////////////
// AWokeAndShootCharacter
AWokeAndShootCharacter::AWokeAndShootCharacter(const class FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UMyCharacterMovementComponent>(
		ACharacter::CharacterMovementComponentName).DoNotCreateDefaultSubobject
             (ACharacter::MeshComponentName))
{
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	CreateCameraComp();

	// Create mesh components
	CreateMeshComps();

	// SetMuzzleLocation
	SetMuzzleLocation();

	// Set Character Movement Component
	CharacterMovement = GetCharacterMovement();

	// Set Health Component
	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	ToggleShotAnim = false;
}

void AWokeAndShootCharacter::BeginPlay()
{ 
	Super::BeginPlay();

	// Attach gun mesh component to Skeleton, doing it here because the skeleton is not yet created in the constructor
	FP_Gun->AttachToComponent(Mesh1P, FAttachmentTransformRules(EAttachmentRule::KeepRelative, false), TEXT("GripPoint"));

    if(auto Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode()))
    {
        Gamemode->PlayersAlive++;
    }
	
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Velocity: %f"), CharacterMovement->GravityScale));
}

void AWokeAndShootCharacter::Tick(float DeltaTime) 
{
	Super::Tick(DeltaTime);
	if(ToggleSpawnAnim)
	{
		FirstPersonCameraComponent->FieldOfView = FMath::Lerp(FirstPersonCameraComponent->FieldOfView,BaseFOV,0.1);
		if(FirstPersonCameraComponent->FieldOfView < BaseFOV+1.f)
		{
			
			ToggleSpawnAnim = false;
		}
	}

	if(ToggleShotAnim)
	{
		// PlayShotAnimation(DeltaTime);
	}
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

	// Bind boost pad detonation events
	PlayerInputComponent->BindAction("DetonateBoostPad", IE_Pressed, this, &AWokeAndShootCharacter::DetonateBoostPad);

	// Bind movement events
	PlayerInputComponent->BindAxis("MoveForward", this, &AWokeAndShootCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AWokeAndShootCharacter::MoveRight);

	// Bind turn events
	PlayerInputComponent->BindAxis("Turn", this, &AWokeAndShootCharacter::LookLeftRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AWokeAndShootCharacter::LookUpDown);
	

	// Setting Sensitivity
	SetCharacterSensitivity();
}

void AWokeAndShootCharacter::Restart() 
{
	Super::Restart();

	if(auto OwnerController = Cast<AWokeAndShootPlayerController>(GetController()))
    {
        OwnerController->ClientReceiveSpawn();
    }

}

void AWokeAndShootCharacter::OnFire()
{

	if(!CanShoot()) {return;}
	
	
	// Network ShotSFX
	PlayShotSound();

	if(!HasAuthority())
	{
		Server_RelayShotSound();
	}
	else
	{
		Multi_RelayShotSound();
	}

	// PlayMuzzleFlashAnimation();
	ToggleShotAnim = true;

	// Prepare parameters for line trace and hit scan
	FVector ViewPointLocation; 
	FRotator ViewPointRotation;
	GetViewPointRotLoc(OUT ViewPointLocation, OUT ViewPointRotation);
	FVector EndPoint = ViewPointLocation + ViewPointRotation.Vector() * Range;

	if (FP_MuzzleLocation == nullptr){return;}	
	FVector SpawnLocation = FP_MuzzleLocation->GetComponentLocation();
	FVector ShotDirection = FP_MuzzleLocation->GetComponentRotation().Vector();
	FRotator SpawnRotation =  FRotationMatrix::MakeFromX(EndPoint - SpawnLocation).Rotator();
	
	if (UWorld* const World = GetWorld())
	{
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(this);

		// Handles HitScan that only runs on the server
		// HitScan(HitResult, Params, ViewPointLocation, EndPoint);
		
		if (bool bOfflineLineTrace = World->LineTraceSingleByChannel(OUT HitResult,ViewPointLocation, EndPoint, ECollisionChannel::ECC_GameTraceChannel2, Params))
		{
			PlayBulletImpactAnimation(HitResult.Location, ShotDirection.Rotation());
			// DrawBulletTracers(ViewPointLocation,SpawnLocation, HitResult.Location, ShotDirection);
	
			Server_RelayHitScan(ViewPointLocation,EndPoint,HitResult);
			if(auto HitBoostPad = Cast<ABoostPad>(HitResult.GetActor()))
			{
				if(HitBoostPad->ClientPrimePad(this))
				{
					if(PrimedBoostPad != nullptr && PrimedBoostPad != HitBoostPad)
					{
						PrimedBoostPad->ClientResetPad();
					}

					PrimedBoostPad = HitBoostPad;
				}
			}
		}

		bCanShoot = false;
		GetWorldTimerManager().SetTimer(ShootingTimerHandle, FTimerDelegate::CreateLambda([this]
		{
        		this->bCanShoot = true;
        		GetWorldTimerManager().ClearTimer(this->ShootingTimerHandle);
		}), ShootingCooldown, false, -2);
		
		// Debug mode
		if(Debug_OnFire)	
		{
			DrawDebugLine(World,ViewPointLocation,HitResult.Location, FColor::Red, true);
			UE_LOG(LogTemp,Warning, TEXT("Hit: %s"), *HitResult.GetComponent()->GetName());
		}
	} 
}

void AWokeAndShootCharacter::DetonateBoostPad() 
{
	if(PrimedBoostPad == nullptr)
	{
		// Play sound effect to notify the players no boost pads are primed.
		return;
	}

	PrimedBoostPad->DetonatePad(this);

	// Network
	// if(!HasAuthority())
	// {
	// 	Server_RelayBoost(PrimedBoostPad);
	// }
	// else
	// {

	// }
	
	// Reseting boost pad slot.
	PrimedBoostPad = nullptr;
}

void AWokeAndShootCharacter::GetViewPointRotLoc(OUT FVector &ViewPointLocation, OUT FRotator &ViewPointRotation) const
{
	AController* OwnerController = GetController();
	if (OwnerController == nullptr) return;

	// OwnerController->GetPlayerViewPoint(OUT ViewPointLocation, OUT ViewPointRotation);
	ViewPointLocation = FirstPersonCameraComponent->GetComponentLocation();
	ViewPointRotation = FirstPersonCameraComponent->GetComponentRotation();
}

// TO DO: Rework, currently inactive
void AWokeAndShootCharacter::DrawBulletTracers(FVector &ViewPointLocation, FVector &SpawnLocation, FVector &HitResultLocation, FVector &ShotDirection)
{
	// Refactor when tracers are complete
	UParticleSystemComponent* TracerComponent;
	if(TracerParticle == nullptr){return;}
	// if (Distance == Range)
	// {
	// 	TracerComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(),TracerParticle,SpawnLocation, ShotDirection.Rotation());
	// }
	else
	{ 		
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(TracerParticle, ProjectileSceneComp);
	}

	// Set Spawn Collision Handling Override
	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	if (TracerComponent)
	{
		FVector DynamicTracerSource = SpawnLocation;
		FVector Target =  HitResultLocation;
		// float OutDistance;
		// FTimerHandle TracerUpdateTH;
		// FTimerDelegate UpdateTracerSourceDelegate = FTimerDelegate::CreateUObject(this,&AWokeAndShootCharacter::UpdateTracerSource, TracerComponent, DynamicTracerSource, Target);
		// TracerComponent->SetFloatParameter(TEXT("Distance"), Distance-500);
		// TracerComponent->GetFloatParameter("Distance", OutDistance);
		// UE_LOG(LogTemp,Warning, TEXT("%f"),Distance);
		TracerComponent->SetBeamSourcePoint(0, DynamicTracerSource, 0);
		TracerComponent->SetBeamTargetPoint(0, HitResultLocation, 0);
		if (TracerComponent->IsActive() == false)
		{
			UE_LOG(LogTemp,Warning, TEXT("TEST2"));
			TracerComponent->SetActive(true, true);
			// UE_LOG(LogTemp,Warning, TEXT("TRACER SET TO ACTIVE, Distance: %f"), Distance);
		}
		DynamicSourceTest = DynamicTracerSource;
		// GetWorld()->GetTimerManager().SetTimer(TracerUpdateTH, UpdateTracerSourceDelegate, 0.08,true);
	}
	else if (TracerComponent->IsActive() == true)
	{
		TracerComponent->SetActive(false, true);
	}
}

// Movement
void AWokeAndShootCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		AddMovementInput(GetActorForwardVector(), Value);
	}
	else if(CharacterMovement->MovementMode.GetValue() == MOVE_Falling && Client_MoveRightAxis != 0)
	{
		// Keep Movement going while air strafing without holding W
		FVector VelocityNoZ = UKismetMathLibrary::LessLess_VectorRotator(GetVelocity(),GetActorRotation());
		if(VelocityNoZ.X > 600)
		{	
			Value = 1.f;
		}	
		else if(VelocityNoZ.X < -600)
		{
			Value = -1.f;
		}

		AddMovementInput(GetActorForwardVector(), Value);
	}
	
	// Set Local Axis Variable
	Client_MoveForwardAxis = Value;

	// Relay Axis to Server
	Server_RelayForwardAxis(Value);
	
}

void AWokeAndShootCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// Restricts left right speed when in air
		if(CharacterMovement->MovementMode.GetValue() == MOVE_Falling)
		{
			Value *= 0.3;
		}

		AddMovementInput(GetActorRightVector(), Value);
	}

	// Set Local Axis Variable
	Client_MoveRightAxis = Value;

	// Relay Axis to Server
	Server_RelayRightAxis(Value);
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

	// Network
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

bool AWokeAndShootCharacter::CanShoot() const
{
	bool canShoot = true;
	
	if (IsDead()) {canShoot = false;}
	if (!bCanShoot) {canShoot = false;}

	return  canShoot;
}

void AWokeAndShootCharacter::DirectionalImpulse(FVector ImpulseDirection) 
{
	CharacterMovement->bIgnoreClientMovementErrorChecksAndCorrection = true;
	CharacterMovement->Launch(ImpulseDirection);
	if(!HasAuthority())
	{
		Server_RelayBoost(ImpulseDirection);
	}


	// CharacterMovement->GravityScale *= 0.9;
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

void AWokeAndShootCharacter::SetCharacterSensitivity() 
{
	if(auto MyGameInstance = Cast<UWnSGameInstance>(GetGameInstance()))
	{
		Sensitivity = MyGameInstance->GetPlayerSensitivity();
	}
}

void AWokeAndShootCharacter::AddFOV(float AdditionalFOV) 
{
	FirstPersonCameraComponent->FieldOfView += AdditionalFOV;
}

void AWokeAndShootCharacter::PawnHandleDeath() 
{
	CharacterMovement->DisableMovement();
	DeathEvent();
}

// Initialization functions
void AWokeAndShootCharacter::CreateCameraComp() 
{
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->FieldOfView = 90.f;
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
				
	// Otherwise won't be visible in the multiplayer
	FP_Gun->bCastDynamicShadow = false;
	FP_Gun->CastShadow = false;
	FP_Gun->SetupAttachment(Mesh1P, TEXT("GripPoint"));
	FP_Gun->SetupAttachment(RootComponent);

	//Create a third person body mesh component
	TP_Body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Third_Person Body"));
	TP_Body->SetOwnerNoSee(true);
	TP_Body->SetupAttachment(RootComponent);	
	
}

void AWokeAndShootCharacter::SetMuzzleLocation() 
{
	FP_MuzzleLocation = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleLocation"));
	FP_MuzzleLocation->SetupAttachment(FP_Gun,TEXT("Muzzle"));
	FP_MuzzleLocation->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
}

// Character actions 
void AWokeAndShootCharacter::PlayShotSound() 
{
	if(IsLocallyControlled())
	{
		// Try and play a firing sound if specified
		if (FireSoundFP != nullptr)
		{
			// UGameplayStatics::PlaySoundAtLocation(this, FireSound, FP_MuzzleLocation->GetComponentLocation());
			UGameplayStatics::SpawnSoundAttached(FireSoundFP,FP_MuzzleLocation,TEXT("Muzzle"));
		}
	}
	else
	{
		if (FireSoundTP != nullptr)
		{
			// UGameplayStatics::PlaySoundAtLocation(this, FireSound, FP_MuzzleLocation->GetComponentLocation());
			UGameplayStatics::SpawnSoundAttached(FireSoundTP,FP_MuzzleLocation,TEXT("Muzzle"));
		}
	}
}

void AWokeAndShootCharacter::PlayShotAnimation(float DeltaTime) 
{
	// Try and play a firing animation if specified
	if(AnimState)
	{
		FirstPersonCameraComponent->FieldOfView = FMath::FInterpConstantTo(FirstPersonCameraComponent->FieldOfView,123.f,DeltaTime,35);
		if(FirstPersonCameraComponent->FieldOfView > 120.9)
			AnimState = false;
	}
	else if(!AnimState)
	{
		FirstPersonCameraComponent->FieldOfView = FMath::FInterpConstantTo(FirstPersonCameraComponent->FieldOfView,120.f,DeltaTime,20);
		if(FirstPersonCameraComponent->FieldOfView < 120.2f)
		{
			ToggleShotAnim = false;
			AnimState = true;
		}
	}
	
	// FirstPersonCameraComponent->FieldOfView-= 5.f;

	// if (FireAnimation != nullptr)
	// {
	// 	if (UAnimInstance* AnimInstance = Mesh1P->GetAnimInstance())
	// 	{
	// 		AnimInstance->Montage_Play(FireAnimation, 1.f);
	// 	}
	// }
}

void AWokeAndShootCharacter::PlayBulletImpactAnimation(FVector HitLocation, FRotator ImpactRotation) 
{
	// Bullet impact
	if(BulletImpact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitLocation, ImpactRotation);
	}

	// Network
	if(!HasAuthority())
	{
		Server_RelayBulletImpact(HitLocation, ImpactRotation);
	}
	else
	{
		Multi_RelayBulletImpact(HitLocation, ImpactRotation);
	}
}

void AWokeAndShootCharacter::PlayMuzzleFlashAnimation() 
{
	if(MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, FP_MuzzleLocation->GetComponentLocation(), FP_MuzzleLocation->GetComponentRotation());
	}
}

void AWokeAndShootCharacter::HitScan(FHitResult& HitResult, FCollisionQueryParams& Params, FVector& StartingLocation, FVector&  EndLocation) 
{
	// Server hitscan
	if(HasAuthority())
	{
		if (bool bLineTrace = GetWorld()->LineTraceSingleByChannel(OUT HitResult, StartingLocation, EndLocation, ECollisionChannel::ECC_GameTraceChannel2, Params))
		{
			if(auto Character = Cast<AWokeAndShootCharacter>(HitResult.GetActor()))
			{
				Multi_RelayDamage(100.f, HitResult.GetActor());
			}
		}
	}
	else
	{
		// Client request hitscan
		// Server_RelayHitScan(StartingLocation, EndLocation);
	}
}

// Network functions
bool AWokeAndShootCharacter::Server_RelayPitch_Validate(float Pitch) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayPitch_Implementation(float Pitch) 
{
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

bool AWokeAndShootCharacter::Server_RelayBulletImpact_Validate(FVector HitLocation, FRotator ShotDirection) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayBulletImpact_Implementation(FVector HitLocation, FRotator ShotDirection) 
{
	// Bullet impact
	if(BulletImpact)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitLocation, ShotDirection);
	}

	Multi_RelayBulletImpact(HitLocation, ShotDirection);
}

bool AWokeAndShootCharacter::Multi_RelayBulletImpact_Validate(FVector HitLocation, FRotator ShotDirection) 
{
	return true;	
}

void AWokeAndShootCharacter::Multi_RelayBulletImpact_Implementation(FVector HitLocation, FRotator ShotDirection) 
{
	if(!IsLocallyControlled())
	{
		// Bullet impact
		if(BulletImpact)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BulletImpact, HitLocation, ShotDirection);
		}
	}
}

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

bool AWokeAndShootCharacter::Server_RelayHitScan_Validate(const FVector& ViewPointLocation, const FVector& EndPoint, const FHitResult& ClientHitResult) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayHitScan_Implementation(const FVector& ViewPointLocation, const FVector& EndPoint, const FHitResult& ClientHitResult) 
{
	Multi_RelayDamage(100.f, ClientHitResult.GetActor());

	// Disabled till server kill verification is made
	// UWorld* World = GetWorld();
	// if(World ==  nullptr)
	// {
	// 	return;
	// }

	// // Server only linetrace
	// FHitResult ServerHitResult;
	// FCollisionQueryParams Params;
	// Params.AddIgnoredActor(this);
	// bool bLineTrace = World->LineTraceSingleByChannel(ServerHitResult, ViewPointLocation, EndPoint,ECollisionChannel::ECC_GameTraceChannel2, Params);
	// if(bLineTrace && ServerHitResult.GetActor())
	// {
		
	// 	if(auto Character = Cast<AWokeAndShootCharacter>(ServerHitResult.GetActor()))
	// 	{
	// 		Multi_RelayDamage(100.f, ServerHitResult.GetActor());
	// 	}
		
	// }
}

bool AWokeAndShootCharacter::Multi_RelayDamage_Validate(float Damage, AActor* HitActor) 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayDamage_Implementation(float Damage, AActor* HitActor) 
{
	
	if(auto Character = Cast<AWokeAndShootCharacter>(HitActor))
	{
		Character->HealthComponent->ApplyDamage(Damage,GetController());
	}
	
}

bool AWokeAndShootCharacter::Server_RelayBoost_Validate(FVector ImpulseDirection) 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayBoost_Implementation(FVector ImpulseDirection)
{
	CharacterMovement->Launch(ImpulseDirection);
	// HitBoostPad->DetonatePad(this);
	// Multi_RelayBoost(HitBoostPad);
}

bool AWokeAndShootCharacter::Server_RelayShotSound_Validate() 
{
	return true;
}

void AWokeAndShootCharacter::Server_RelayShotSound_Implementation() 
{
	// Run on server.
	PlayShotSound();
	// Replicate to clients.
	Multi_RelayShotSound();
}

bool AWokeAndShootCharacter::Multi_RelayShotSound_Validate() 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayShotSound_Implementation() 
{
	if(!IsLocallyControlled())
	{
		PlayShotSound();
	}
}

bool AWokeAndShootCharacter::Multi_RelayBoost_Validate(FVector ImpulseDirection) 
{
	return true;
}

void AWokeAndShootCharacter::Multi_RelayBoost_Implementation(FVector ImpulseDirection) 
{
	// if(!IsLocallyControlled())
	// {
	// 	HitBoostPad->DetonatePad(this);
	// }
}
