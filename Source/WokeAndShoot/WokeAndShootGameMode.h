// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WokeAndShoot/WokeAndShootCharacter.h"
#include "WokeAndShootGameMode.generated.h"

struct PlayerInformation
{
	FString PlayerName;
	int32 Score = 0;
};

UCLASS(minimalapi)
class AWokeAndShootGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	FTimerHandle DespawnBodyTH;

private:
	UFUNCTION()
	void DespawnBody(APawn* PlayerPawn);
protected:
	bool GameOver = false;

protected:

	virtual void BeginPlay()override;
	void UpdateScore(uint32 KillerID);
	void RestartGame();
public:
	UPROPERTY(EditAnywhere)
	int32 MaxScore = 2;
	UPROPERTY(EditAnywhere)
	bool ScoreLimit = true;

	int32 PlayersAlive = 0;
	int32 PlayersOnline = 0;
	TArray<FVector> SpawnLocations;
	TMap<uint32,PlayerInformation> Players;
public:
public:
	AWokeAndShootGameMode();

	virtual void PawnKilled(APawn* PlayerPawn, AController* Killer);
};



