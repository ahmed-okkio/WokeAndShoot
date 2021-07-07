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

	TArray<AWokeAndShootCharacter*> CooldownList;

	TArray<FTimerHandle> TimerHandles;

	FTimerDelegate PopDelegate;

private:

	void RemoveActorCD();
	
	void InitiateCooldown(AWokeAndShootCharacter* HitActor);

	bool WithinConeRange(FVector& PadLocation, FVector& HitActorLocation);

	FVector GetImpulseDirection(FVector& ActorLocation);

	void ApplyBoost(AWokeAndShootCharacter* Initiator);

	// UFUNCTION(Server,Reliable,WithValidation)
	// void Server_RelayBoost();
	// bool Server_RelayBoost_Validate();
	// void Server_RelayBoost_Implementation();

	// UFUNCTION(NetMulticast,Reliable,WithValidation)
	// void Multi_RelayBoost();
	// bool Multi_RelayBoost_Validate();
	// void Multi_RelayBoost_Implementation();

	

public:	
	// Sets default values for this actor's properties
	ABoostPad();

	void BoostPlayers(AWokeAndShootCharacter* Initiator);
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//OnHit Comp
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoostForce)
	FVector HeightOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = BoostForce)
	float BoostAmount = 2000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Cooldown)
	float BoostPadCooldown = 3.f;


};
