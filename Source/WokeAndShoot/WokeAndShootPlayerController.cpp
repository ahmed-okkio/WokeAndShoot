// Fill out your copyright notice in the Description page of Project Settings.

#include "WokeAndShootPlayerController.h"
#include "WokeAndShoot/WokeAndShootGameMode.h"
#include "WokeAndShoot/ServerComponents/MyPlayerState.h"
#include "WokeAndShoot/GameComponents/DeathScreenWidget.h"
#include "WokeAndShoot/WokeAndShootCharacter.h"
#include "WokeAndShoot/DevTools/MyReadWriteHelper.h"

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

void AWokeAndShootPlayerController::DisplayDeadWidget(FString KilledBy) 
{
    if(!IsLocalPlayerController()){return;}
    
    if(DeathScreen = Cast<UDeathScreenWidget>(CreateWidget(this, DeathScreenClass)))
    {
        DeathScreen->KillerName = KilledBy;
        DeathScreen->AddToViewport();
    }  
}

void AWokeAndShootPlayerController::ClearDeadWidget() 
{
    if(!IsLocalPlayerController()){return;}
    if(DeathScreen != nullptr)
    {
        DeathScreen->RemoveFromViewport();
    }
}

AWokeAndShootPlayerController::AWokeAndShootPlayerController() 
{
    PlayerName = MyReadWriteHelper::LoadFileToString("Username.cfg","UserSettings");
}

void AWokeAndShootPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{
    Super::EndPlay(EndPlayReason);

    if(AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode()))
    {
        Gamemode->PlayersOnline--;
        Gamemode->Players.Remove(GetUniqueID());
    }
}
