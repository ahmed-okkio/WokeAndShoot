// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "../../GameComponents/PlayerController/WokeAndShootPlayerController.h"
#include "MyPlayerState.h"


void AMyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AMyPlayerState, LastKilledBy, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AMyPlayerState, NewPawn, SharedParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(AMyPlayerState, CurrentKillInfo, SharedParams);
    
}

void AMyPlayerState::OnRep_KilledBy() 
{

    if(auto PlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()))
    {
        // PlayerController->ClientReceiveDeath();
    }
}

void AMyPlayerState::OnRep_Respawn() 
{
    if(auto PlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()))
    {
        PlayerController->ClientReceiveSpawn();
    }
}

void AMyPlayerState::OnRep_KillFeed() 
{
    if(auto PlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()))
    {
        PlayerController->ClientReceiveKillInfo(CurrentKillInfo);
        if(GetPlayerName() == CurrentKillInfo.KilledName)
        {
            PlayerController->ClientReceiveDeath();
        }
    }
}

