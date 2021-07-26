// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootGameMode.h"
#include "GameFramework/GameSession.h"
#include "WokeAndShootHUD.h"
#include "WokeAndShootCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWokeAndShootGameMode::AWokeAndShootGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWokeAndShootHUD::StaticClass();
}

void AWokeAndShootGameMode::PawnKilled(APawn* PlayerPawn, AController* Killer) 
{
	//Despawning body
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this,FName("DespawnBody"), PlayerPawn);
	GetWorld()->GetTimerManager().SetTimer(DespawnBodyTH, TimerDel, 3.f, false);
	UpdateScore(Killer->GetUniqueID());
}

void AWokeAndShootGameMode::BeginPlay() 
{
}

void AWokeAndShootGameMode::UpdateScore(uint32 KillerID) 
{
	int32 CurrentPlayerScore = Players.Find(KillerID)->Score;
	CurrentPlayerScore++;
	Players.Find(KillerID)->Score = CurrentPlayerScore;
	if(ScoreLimit && CurrentPlayerScore == MaxScore)
	{
		GameOver = true;
		RestartGame();
	}
}

void AWokeAndShootGameMode::RestartGame() 
{
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		APlayerController* PlayerController = Iterator->Get();
		if(PlayerController == nullptr)
		{
			return;
		}
		APawn* PlayerPawn = PlayerController->GetPawn();
		if(PlayerPawn != nullptr)
		{
			PlayerPawn->DetachFromControllerPendingDestroy();
			PlayerPawn->Destroy();
		}
	}
}

void AWokeAndShootGameMode::DespawnBody(APawn* PlayerPawn) 
{
	PlayerPawn->DetachFromControllerPendingDestroy();
	PlayerPawn->Destroy();
}
