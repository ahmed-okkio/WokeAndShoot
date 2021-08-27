// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "WokeAndShootCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UMotionControllerComponent;
class UAnimMontage;
class USoundBase;
class UParticleSystem;
class UHealthComponent;

UCLASS(config=Game)
class AWokeAndShootCharacter : public ACharacter
{
	GENERATED_BODY()
private:
	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* TP_Body;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	UPROPERTY(EditAnywhere, Category = Particles)
	UParticleSystem* TracerParticle;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	UPROPERTY(EditAnywhere, Category = Animations)
	UParticleSystem* BulletImpact;

	//Shooting Range
	UPROPERTY(EditAnywhere, Category = Gameplay)
	float Range = 10000.f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	float StrafeMultiplier = 2.f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
	FVector FullScale = FVector(1,1,1);

	FVector DynamicSourceTest;

	// AWokeAndShootProjectile* Projectile;
	USceneComponent* ProjectileSceneComp;

	FVector WishDir;

private:
	// Initialization
	void CreateCameraComp();
	void CreateMeshComps();
	void SetMuzzleLocation();

	// Actions
	void PlayShotSound();
	void PlayShotAnimation();
	void PlayBulletImpactAnimation(FVector HitLocation, FRotator ImpactRotation);
	void HitScan(FHitResult& HitResult, FCollisionQueryParams& Params, FVector& StartingLocation, FVector&  EndLocation);

	// Network
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayPitch(float Pitch);
	bool Server_RelayPitch_Validate(float Pitch);
	void Server_RelayPitch_Implementation(float Pitch);

	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void Multi_RelayPitch(float Pitch);
	bool Multi_RelayPitch_Validate(float Pitch);
	void Multi_RelayPitch_Implementation(float Pitch);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayBulletImpact(FVector HitLocation, FRotator ShotDirection);
	bool Server_RelayBulletImpact_Validate(FVector HitLocation, FRotator ShotDirection);
	void Server_RelayBulletImpact_Implementation(FVector HitLocation, FRotator ShotDirection);

	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void Multi_RelayBulletImpact(FVector HitLocation, FRotator ShotDirection);
	bool Multi_RelayBulletImpact_Validate(FVector HitLocation, FRotator ShotDirection);
	void Multi_RelayBulletImpact_Implementation(FVector HitLocation, FRotator ShotDirection);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayForwardAxis(float MoveForwardAxisParam);
	bool Server_RelayForwardAxis_Validate(float MoveForwardAxisParam);
	void Server_RelayForwardAxis_Implementation(float MoveForwardAxisParam);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayRightAxis(float MoveRightAxisParam);
	bool Server_RelayRightAxis_Validate(float MoveRightAxisParam);
	void Server_RelayRightAxis_Implementation(float MoveRightAxisParam);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayHitScan(const FVector& ViewPointLocation, const FVector& EndPoint);
	bool Server_RelayHitScan_Validate(const FVector& ViewPointLocation, const FVector& EndPoint);
	void Server_RelayHitScan_Implementation(const FVector& ViewPointLocation, const FVector& EndPoint);

	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void Multi_RelayDamage(float Damage, AActor* HitActor);
	bool Multi_RelayDamage_Validate(float Damage, AActor* HitActor);
	void Multi_RelayDamage_Implementation(float Damage, AActor* HitActor);

	UFUNCTION(Server,Reliable,WithValidation)
	void Server_RelayBoost(ABoostPad* HitBoostPad);
	bool Server_RelayBoost_Validate(ABoostPad* HitBoostPad);
	void Server_RelayBoost_Implementation(ABoostPad* HitBoostPad);

	UFUNCTION(NetMulticast,Reliable,WithValidation)
	void Multi_RelayBoost(ABoostPad* HitBoostPad);
	bool Multi_RelayBoost_Validate(ABoostPad* HitBoostPad);
	void Multi_RelayBoost_Implementation(ABoostPad* HitBoostPad);

	// UFUNCTION(NetMulticast,Reliable,WithValidation)
	// void Multi_RelayDeath(float Pitch);
	// bool Multi_RelayDeath_Validate(float Pitch);
	// void Multi_RelayDeath_Implementation(float Pitch);




protected:

	virtual void BeginPlay();

	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	//Get ViewPoint Rotation and Location
	void GetViewPointRotLoc(FVector &ViewPointLocation, FRotator &ViewPointRotation) const;

	/** Fires a projectile. */
	void OnFire();

	//Draw Bullet Tracers
	void DrawBulletTracers(FVector &ViewPointLocation, FVector &SpawnLocation,FVector &HitResultLocation, FVector &ShotDirection, float Distance);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

	void JumpHandler(float Val);

	void HandleDeath();

	/**
	 * Called via input to turn at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpDown(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookLeftRight(float Rate);

public:

	UPROPERTY(EditAnywhere, Category=Camera)
	float Sensitivity = 0.5;

	/** Gun muzzle's offset from the characters location */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	FVector GunOffset;

	/** Projectile class to spawn */
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AWokeAndShootProjectile> ProjectileClass;

	/** Sound to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Gameplay)
	USoundBase* FireSound;

	/** AnimMontage to play each time we fire */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UAnimMontage* FireAnimation;

	float Client_MoveRightAxis;
	float Client_MoveForwardAxis;

	UHealthComponent* HealthComponent;

	//CharacterMovement
	UCharacterMovementComponent* CharacterMovement;

	bool bBoosting = false;

	// FString Killer = TEXT("");

	bool bIsDead = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Respawn)
	FName KilledBy = "Unknown";


public:

	AWokeAndShootCharacter(const class FObjectInitializer& ObjectInitializer);

	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }
	/** Returns HealthComponent subobject **/
	UHealthComponent* GetHealthComponent() const {return HealthComponent; }

	//Apply impulse to a direction
	void DirectionalImpulse(FVector ImpulseDirection);

	//Tick Function
	virtual void Tick(float DeltaTime)override;

	virtual void Landed(const FHitResult & Hit) override;

	UFUNCTION(BlueprintPure)
	bool IsDead() const;

public:
///////////////////////////////////////////////////////////////////////////////////////
// Debug toggles
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Mode")
	bool Debug_OnFire = false;

};

