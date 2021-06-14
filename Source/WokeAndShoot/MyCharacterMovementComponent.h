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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "My Character Movement|Air Strafing", Meta = (AllowPrivateAccess = "true"))
	float StrafeMultiplier = 0.5f;

#pragma endregion

#pragma region Overrides
public:
	virtual void PhysFalling(float deltaTime, int32 Iterations) override;

#pragma endregion
};
