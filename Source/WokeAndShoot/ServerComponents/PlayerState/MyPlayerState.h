// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */

USTRUCT()
struct FKillInfo
{
	GENERATED_BODY();
	UPROPERTY()
	FString KillerName;
	UPROPERTY()
	FString KilledName;
	UPROPERTY()
	int32 KilledPlayerId;
	UPROPERTY()
	uint8 KillCount;
};
UCLASS()
class WOKEANDSHOOT_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()
	

private:
	UFUNCTION()
	void OnRep_KilledBy();
	UFUNCTION()
	void OnRep_Respawn();
	UFUNCTION()
	void OnRep_KillFeed();

public:

	UPROPERTY(ReplicatedUsing = OnRep_KilledBy )
	int32 NetworkPlayerId;
	UPROPERTY(ReplicatedUsing = OnRep_Respawn )
	APawn* NewPawn = nullptr;
	UPROPERTY(ReplicatedUsing = OnRep_KillFeed)
	FKillInfo CurrentKillInfo;
	

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
