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

UCLASS(config=Game)
class AWokeAndShootCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** Gun mesh: 1st person view (seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* FP_Gun;

	/** Location on gun mesh where projectiles should spawn. */
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USceneComponent* FP_MuzzleLocation;

	UPROPERTY(EditAnywhere, Category = Particles)
	UParticleSystem* TracerParticle;
	

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;
private:

	UPROPERTY(EditAnywhere, Category = Animations)
		UParticleSystem* BulletImpact;
		//Shooting Range
	UPROPERTY(EditAnywhere, Category = Gameplay)
		float Range = 5000.f;

	UPROPERTY(EditAnywhere, Category = Gameplay)
		FVector FullScale = FVector(1,1,1);

		FVector DynamicSourceTest;
	

public:
	AWokeAndShootCharacter();

protected:
	virtual void BeginPlay();

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



protected:
	

	//Update Tracer Source
	void UpdateTracerSource(UParticleSystemComponent* TracerComponent, FVector DynamicTracerSource, FVector Target);

	//Get ViewPoint Rotation and Location
	void GetViewPointRotLoc(FVector &ViewPointLocation, FRotator &ViewPointRotation);

	/** Fires a projectile. */
	void OnFire();

	//Draw Bullet Tracers
	void DrawBulletTracers(FVector &ViewPointLocation, FVector &SpawnLocation,FVector &HitResultLocation, FVector &ShotDirection, float Distance);

	/** Handles moving forward/backward */
	void MoveForward(float Val);

	/** Handles stafing movement, left and right */
	void MoveRight(float Val);

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
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface


public:
	/** Returns Mesh1P subobject **/
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }
	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

};

