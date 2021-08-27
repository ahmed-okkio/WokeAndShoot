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
	float StrafeSharpness = 0.01;
	

#pragma endregion

#pragma region Overrides
public:
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

#pragma endregion
public:
///////////////////////////////////////////////////////////////////////////////////////
// Debug toggles

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Mode")
	bool Debug_AirStrafing = false;
};
