// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WokeAndShootPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API AWokeAndShootPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UDeathScreenWidget> DeathScreenClass;

	UDeathScreenWidget* DeathScreen = nullptr;

private:
	AWokeAndShootPlayerController();
	~AWokeAndShootPlayerController();


protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	FString PlayerName = TEXT("Player");
	// FString KilledBy = TEXT("Unknown");
public:
	// void GameHasEnded();
	void PlayerKilled();

	void DisplayDeadWidget(FString KilledBy);
	void PossessNewPawn(APawn* NewPawn);
};
