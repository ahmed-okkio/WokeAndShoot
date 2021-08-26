// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/UnrealNetwork.h"
#include "WokeAndShoot/WokeAndShootPlayerController.h"
#include "MyPlayerState.h"


void AMyPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const 
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);


    FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(AMyPlayerState, LastKilledBy, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AMyPlayerState, NewPawn, SharedParams);
    
}

void AMyPlayerState::OnRep_KilledBy() 
{
    if(AWokeAndShootPlayerController* PlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()))
    {
        PlayerController->DisplayDeadWidget(LastKilledBy);
    }
}

void AMyPlayerState::OnRep_Respawn() 
{
    if(AWokeAndShootPlayerController* PlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()))
    {
        PlayerController->ClearDeadWidget();
    }
}
