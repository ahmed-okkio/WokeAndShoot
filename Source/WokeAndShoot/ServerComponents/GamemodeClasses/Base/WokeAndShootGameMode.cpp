// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameFramework/GameSession.h"
#include "UObject/ConstructorHelpers.h"
#include "WokeAndShoot/ServerComponents/PlayerState/MyPlayerState.h"
#include "../../../GameComponents/Character/WokeAndShootCharacter.h"
#include "../../../GameComponents/PlayerController/WokeAndShootPlayerController.h"
#include "WokeAndShootGameMode.h"

AWokeAndShootGameMode::AWokeAndShootGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/WokeAndShoot/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;
}

void AWokeAndShootGameMode::PawnKilled(AController* Killed, AController* Killer) 
{
	AWokeAndShootPlayerController* KillerController = Cast<AWokeAndShootPlayerController>(Killer);
	if(KillerController == nullptr){return;}

	AWokeAndShootPlayerController* KilledController = Cast<AWokeAndShootPlayerController>(Killed);
	if(KilledController == nullptr){return;}

	if(KilledController->GetPlayerIsDead()){return;}
		

	//Despawning body
	FTimerDelegate TimerDel;
	TimerDel.BindUFunction(this,FName("DespawnBody"), KilledController);
	GetWorld()->GetTimerManager().SetTimer(DespawnBodyTH, TimerDel, 3.f, false);
	
	if(KilledController->HasAuthority())
	{
		KilledController->SetPlayerIsDead(true);

	}

	UpdateKillerName(KilledController, KillerController);
	UpdateScore(Killer);
	// KilledController->UnPossess();
	// Temporary patch to notify server player he has been killed.
	if(KilledController->IsLocalPlayerController())
	{
		KilledController->ClientReceiveDeath();
	}
}

void AWokeAndShootGameMode::BeginPlay() 
{
	Super::BeginPlay();
}

void AWokeAndShootGameMode::UpdateKillerName(AWokeAndShootPlayerController* KilledController, AWokeAndShootPlayerController* KillerController) 
{
	if(auto KilledPlayerState = KilledController->GetPlayerState<AMyPlayerState>())
	{
		if(auto KillerPlayerState = KillerController->PlayerState)
		{
			KillInfo CurrentKillInfo {KillerPlayerState->GetPlayerName(),KilledPlayerState->GetPlayerName()};
			ServerKillCount++;
			KillInfoList.Add(CurrentKillInfo);
			PushKillFeedToPlayers(CurrentKillInfo);
			// KilledPlayerState->LastKilledBy = KillerPlayerState->GetPlayerName();
			KilledPlayerState->ForceNetUpdate();

		}
	}
}

void AWokeAndShootGameMode::UpdateScore(AController* Killer) 
{
	uint32 KillerID = Killer->GetUniqueID();

	Killer->GetPlayerState<AMyPlayerState>()->Score++;
	int32 CurrentPlayerScore = Players.Find(KillerID)->Score;
	CurrentPlayerScore++;

	Players.Find(KillerID)->Score = CurrentPlayerScore;

	if(ScoreLimit && CurrentPlayerScore == MaxScore)
	{
		GameOver = true;
		EndGame();
	}
}

void AWokeAndShootGameMode::EndGame() 
{
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		if(auto PlayerController = Cast<AWokeAndShootPlayerController>(Iterator->Get()))
		{	
			PlayerController->Multi_ClientEndGame();
		}
	}
	FTimerHandle MyTH;
	GetWorldTimerManager().SetTimer(MyTH, this, &AWokeAndShootGameMode::RestartGame, 25.0f, false);
}

void AWokeAndShootGameMode::RestartGame() 
{
	GameOver = false;
	ResetLevel();
	RestartAllPlayerControllers();
}

void AWokeAndShootGameMode::DespawnBody(AWokeAndShootPlayerController* Killed) 
{
	if(GameOver){return;}

    PlayersAlive--;

	if(APawn* KilledPawn = Killed->GetPawn())
	{
		KilledPawn->DetachFromControllerPendingDestroy();
		KilledPawn->Destroy();
	}

	Respawn(Killed);
}
 
void AWokeAndShootGameMode::Respawn(AWokeAndShootPlayerController* PlayerController) 
{
	if(GameOver){return;}

    //Replace with optimal spawn algo
    int32 SpawnIndex = FMath::RandRange(0,SpawnLocations.Num()-1);
    FVector SpawnLocation = SpawnLocations[SpawnIndex];
    FRotator SpawnRotation = FRotator(0,0,0);
    FActorSpawnParameters SpawnParams;

    if(auto PlayerCharacter = GetWorld()->SpawnActor<AWokeAndShootCharacter>(DefaultPawnClass,SpawnLocation,SpawnRotation))
    {
		PlayerController->SetPlayerIsDead(false);

        PlayerController->GetPlayerState<AMyPlayerState>()->NewPawn = PlayerCharacter;

		// Possess new pawn(respawn)
		PlayerController->Possess(PlayerCharacter);

		//Only for when playing on the server as a client
		if(PlayerController->IsLocalPlayerController())
		{
			PlayerController->ClientReceiveSpawn();
		}
    }
	else
	{
		Respawn(PlayerController);
	}
}

void AWokeAndShootGameMode::PushKillFeedToPlayers(KillInfo NewKillInfo) 
{
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{

		if( auto PlayerController = Cast<AWokeAndShootPlayerController>(Iterator->Get()))
		{
			if(auto PlayerState = PlayerController->GetPlayerState<AMyPlayerState>())
			{
				PlayerState->CurrentKillInfo = FKillInfo {NewKillInfo.KillerName,NewKillInfo.KilledName,ServerKillCount};
				PlayerController->ForceNetUpdate();

				//Call for server player to receive KillInfo
				if(PlayerController->IsLocalPlayerController() && PlayerController->HasAuthority())
				{
					PlayerController->ClientReceiveKillInfo(FKillInfo {NewKillInfo.KillerName,NewKillInfo.KilledName,ServerKillCount});
				}
			}
     	}
}
}

void AWokeAndShootGameMode::RestartAllPlayerControllers() 
{
	for( FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator )
	{
		if(auto PlayerController = Cast<AWokeAndShootPlayerController>(Iterator->Get()))
		{
			if(APawn* PlayerPawn = PlayerController->GetPawn())
			{
				PlayerPawn->DetachFromControllerPendingDestroy();
				PlayerPawn->Destroy();
			}

			Respawn(PlayerController);

			PlayerController->Multi_ClientRestartGame();
		}
	}
}
