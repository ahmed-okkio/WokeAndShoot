// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API UMyCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
#pragma region Defaults
private:
	//Variable that controls how much speed air strafing will provide
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	float StrafeMultiplier = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	//How sharp your velocity conforms to your forward vector
	float StrafeSharpness = 0.01f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	// How much time is allowed until air max speed is reset back to normal
	float AirStrafeGracePeriod = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	// The max attainable speed while in air
	float MaxAirSpeed = 2400.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	// Amount to change FOV by while accelerating
	float DynamicFOVAmount = 20.f;

	FTimerHandle TH_AirStrafeReset;

	bool DynamicFOVSwitch = true;

private:
	void ResetAirMaxSpeed();
	void DynamicFOV();

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
#pragma endregion

#pragma region Overrides
public:
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;
	virtual void PhysWalking(float deltaTime, int32 Iterations);

#pragma endregion
public:
///////////////////////////////////////////////////////////////////////////////////////
// Debug toggles

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Mode")
	bool Debug_AirStrafing = false;
};
