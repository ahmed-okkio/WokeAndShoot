// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoostPad.generated.h"
class USphereComponent;
class URadialForceComponent;
class AWokeAndShootCharacter;
UCLASS()
class WOKEANDSHOOT_API ABoostPad : public AActor
{
	GENERATED_BODY()


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* PadMesh;

	UPROPERTY(VisibleDefaultsOnly, Category=Pad)
	USphereComponent* CollisionComp;
	UPROPERTY(EditAnywhere)
	UMaterialInstance* DefaultMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* PrimedMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInstance* DetonatedMaterial;

	TArray<AWokeAndShootCharacter*> CooldownList;

	// TArray<FTimerHandle> TimerHandles;

	FTimerHandle TH_PadTimeOutTimer;

	FTimerDelegate PopDelegate;

private:

	void RemoveActorCD();
	
	void InitiateCooldown(AWokeAndShootCharacter* HitActor);

	bool WithinConeRange(FVector& PadLocation, FVector& HitActorLocation);

	FVector GetImpulseDirection(FVector& ActorLocation);

	


protected:
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoostForce)
	FVector HeightOffset;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoostForce)
	float BoostAmount = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	float BoostPadCooldown = 3.f;

public:	
	ABoostPad();

	void DetonatePad(AWokeAndShootCharacter* Initiator);

	// Attempts to prime this pad if the initiator is not on cooldown and returns true if successfully primed. otherwise false.
	bool ClientPrimePad(AWokeAndShootCharacter* Initiator);

	void ClientResetPad();
};
