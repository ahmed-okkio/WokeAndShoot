// Fill out your copyright notice in the Description page of Project Settings.


#include "../PlayerController/WokeAndShootPlayerController.h"
#include "../ConfigFiles/GameConfig.h"
#include "WnSGameInstance.h"
#include "WokeAndShoot/DevTools/MyReadWriteHelper.h"

UWnSGameInstance::UWnSGameInstance() 
{
    // PlayerData = CreateDefaultSubobject<AGameConfig>(TEXT("WnsPlayerConfig"));
}

void UWnSGameInstance::Init() 
{
    Super::Init();
    PlayerData = NewObject<AGameConfig>();
}

void UWnSGameInstance::Shutdown() 
{
    Super::Shutdown();

    // Save changes on shutdown
    if(!LoadedPlayerData.PlayerName.IsEmpty() && LoadedPlayerData.PlayerSensitivity >= 0)
    {
        PlayerData->Cfg_PlayerName = LoadedPlayerData.PlayerName;
        PlayerData->Cfg_PlayerSensitivity = LoadedPlayerData.PlayerSensitivity;
        PlayerData->Cfg_LastServerIP = LastServerIP;
        PlayerData->SaveConfig();
    }
}

void UWnSGameInstance::StartGameInstance() 
{
    Super::StartGameInstance();
}

FString UWnSGameInstance::GetPlayerName() const
{
    return LoadedPlayerData.PlayerName;
}
void UWnSGameInstance::SetPlayerName(FString& NewPlayerName) 
{
    LoadedPlayerData.PlayerName = *NewPlayerName;
}

float UWnSGameInstance::GetPlayerSensitivity() const
{
    return  LoadedPlayerData.PlayerSensitivity;
}

void UWnSGameInstance::SetPlayerSensitvity(float NewPlayerSensitivity) 
{
    LoadedPlayerData.PlayerSensitivity = NewPlayerSensitivity;
}

FString UWnSGameInstance::GetSavedIP() const
{
    return LastServerIP;
}

void UWnSGameInstance::SetSavedIP(const FString& NewServerIP)
{
    LastServerIP = NewServerIP;
}

void UWnSGameInstance::LoadPlayerData() 
{
    if(PlayerData->Cfg_PlayerName.IsEmpty())
	{
		PlayerData->Cfg_PlayerName = "Player";
	}
	if(PlayerData->Cfg_PlayerSensitivity <= 0.f)
	{
		PlayerData->Cfg_PlayerSensitivity = 1.f;
	}
	SaveConfig();

    LoadedPlayerData.PlayerName = PlayerData->Cfg_PlayerName;
    LoadedPlayerData.PlayerSensitivity = PlayerData->Cfg_PlayerSensitivity;
    LastServerIP = PlayerData->Cfg_LastServerIP;
}
