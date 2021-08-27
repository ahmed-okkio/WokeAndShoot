// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SpawnLocation.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API ASpawnLocation : public APlayerStart
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	
public:
	ASpawnLocation(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere)
	TSubclassOf<ASpawnLocation> SpawnLocations;
};
