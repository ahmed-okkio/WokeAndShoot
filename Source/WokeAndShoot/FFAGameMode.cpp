// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/GameSession.h"
#include "FFAGameMode.h"




AFFAGameMode::AFFAGameMode() 
{
    
}
void AFFAGameMode::PawnKilled(APawn* Pawn, AController* Killer) 
{
    AController* PlayerController = Pawn->GetController();
    PlayersAlive--;
	if(!GameOver && PlayerController != nullptr)
	{
        Respawn(PlayerController);
	}
    Super::PawnKilled(Pawn, Killer);
}


void AFFAGameMode::Respawn(AController* PlayerController) 
{
     //Replace with optimal spawn algo
    int32 SpawnIndex = FMath::RandRange(0,SpawnLocations.Num()-1);
    FVector SpawnLocation = SpawnLocations[SpawnIndex];
    FRotator SpawnRotation = FRotator(0,0,0);
    FActorSpawnParameters SpawnParams;
    AWokeAndShootCharacter* PlayerCharacter = GetWorld()->SpawnActor<AWokeAndShootCharacter>(DefaultPawnClass,SpawnLocation,SpawnRotation);
    if(PlayerCharacter != nullptr)
    {
        PlayerController->Possess(PlayerCharacter);
    }
}
