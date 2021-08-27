// Fill out your copyright notice in the Description page of Project Settings.


#include "../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "SpawnLocation.h"


ASpawnLocation::ASpawnLocation(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    
}

void ASpawnLocation::BeginPlay() 
{
    AWokeAndShootGameMode* Gamemode = GetWorld()->GetAuthGameMode<AWokeAndShootGameMode>();
    if(Gamemode != nullptr)
    {
        Gamemode->SpawnLocations.Add(GetActorLocation());
    }
}
