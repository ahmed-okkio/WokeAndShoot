// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Base/WokeAndShootGameMode.h"
#include "FFAGameMode.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API AFFAGameMode : public AWokeAndShootGameMode
{
	GENERATED_BODY()
	
public:
	AFFAGameMode();
	virtual void PawnKilled(AController* Killed, AController* Killer) override;

};
