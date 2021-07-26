// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WokeAndShootGameMode.h"
#include "FFAGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API AFFAGameMode : public AWokeAndShootGameMode
{
	GENERATED_BODY()
private:

private:
	void Respawn(AController* PlayerController);
public:
public:
	AFFAGameMode();
	virtual void PawnKilled(APawn* Pawn, AController* Killer) override;

};
