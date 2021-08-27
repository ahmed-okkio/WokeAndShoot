// Fill out your copyright notice in the Description page of Project Settings.

#include "GameFramework/GameSession.h"
#include "FFAGameMode.h"

AFFAGameMode::AFFAGameMode() 
{
    
}
void AFFAGameMode::PawnKilled(AController* Killed, AController* Killer) 
{
    Super::PawnKilled(Killed, Killer);
}



