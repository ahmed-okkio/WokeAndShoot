// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WokeAndShootGameMode.generated.h"

struct PlayerInformation
{
	FString PlayerName;
	int32 Score = 0;
};
struct KillInfo
{
	FString KillerName;
	FString KilledName;
};

UCLASS(minimalapi)
class AWokeAndShootGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	FTimerHandle DespawnBodyTH;
	TArray<KillInfo> KillInfoList;
	uint8 ServerKillCount;

private:
	UFUNCTION()
	void DespawnBody(AWokeAndShootPlayerController* KilledController);
	void Respawn(AWokeAndShootPlayerController* PlayerController);
	void PushKillFeedToPlayers(KillInfo NewKillInfo);

protected:
	bool GameOver = false;

protected:
	virtual void BeginPlay()override;
	void UpdateKillerName(AWokeAndShootPlayerController* KilledController, AWokeAndShootPlayerController* KillerController);
	void UpdateScore(AController* Killer);
	void RestartGame();

public:
	UPROPERTY(EditAnywhere)
	int32 MaxScore = 2;
	UPROPERTY(EditAnywhere)
	bool ScoreLimit = true;
	UPROPERTY(EditAnywhere)
	float RespawnTimer = 3.f;

	int32 PlayersAlive = 0;
	int32 PlayersOnline = 0;
	TArray<FVector> SpawnLocations;
	TMap<uint32,PlayerInformation> Players;
	
public:
	AWokeAndShootGameMode();

	virtual void PawnKilled(AController* Killed, AController* Killer);
};



