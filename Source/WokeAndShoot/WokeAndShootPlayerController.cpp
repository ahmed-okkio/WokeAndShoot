// Fill out your copyright notice in the Description page of Project Settings.

#include "WokeAndShootPlayerController.h"
#include "WokeAndShoot/WokeAndShootGameMode.h"
#include "WokeAndShoot/DevTools/MyReadWriteHelper.h"


void AWokeAndShootPlayerController::GameHasEnded() 
{
    //Show game end UI
}


AWokeAndShootPlayerController::AWokeAndShootPlayerController() 
{
    PlayerName = MyReadWriteHelper::LoadFileToString("Username.cfg","UserSettings");
}

AWokeAndShootPlayerController::~AWokeAndShootPlayerController() 
{
     
}



void AWokeAndShootPlayerController::BeginPlay() 
{
    Super::BeginPlay();
    AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode());
    if(Gamemode != nullptr)
    {
        PlayerInformation CurrentPlayer {PlayerName};
        Gamemode->PlayersOnline++;
        Gamemode->Players.Add(GetUniqueID(),CurrentPlayer);
    }
}

void AWokeAndShootPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{
    Super::EndPlay(EndPlayReason);
    AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode());
    if(Gamemode != nullptr)
    {
        Gamemode->PlayersOnline--;
        Gamemode->Players.Remove(GetUniqueID());
    }
}
