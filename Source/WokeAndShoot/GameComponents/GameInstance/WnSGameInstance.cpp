// Fill out your copyright notice in the Description page of Project Settings.


#include "../ConfigFiles/GameConfig.h"
#include "WnSGameInstance.h"


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
    SaveToFile();
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

int UWnSGameInstance::GetFPSLimit() const
{
   return LoadedPlayerData.FPSLimit; 
}

void UWnSGameInstance::SetFPSLimit(const int& NewFPSLimit)
{
    LoadedPlayerData.FPSLimit = NewFPSLimit;
}

void UWnSGameInstance::LoadPlayerData() 
{
    
    
    if(PlayerData->Cfg_PlayerName.IsEmpty()) PlayerData->Cfg_PlayerName = "Player";
	if(PlayerData->Cfg_PlayerSensitivity <= 0.f) PlayerData->Cfg_PlayerSensitivity = 1.f;
	if(PlayerData->Cfg_FPSLimit < 0)
	{
	    // get max refresh rate
	    FScreenResolutionArray Resolutions = FScreenResolutionArray{};
	    RHIGetAvailableResolutions(Resolutions, false);
        
	    uint32 MaxRefreshRate = 0;
	    for (const auto Res : Resolutions)
	    {
	        if(Res.RefreshRate > MaxRefreshRate) MaxRefreshRate = Res.RefreshRate;
	    }
	    
	    PlayerData->Cfg_FPSLimit = MaxRefreshRate;
	}
	SaveConfig();

    LoadedPlayerData.PlayerName = PlayerData->Cfg_PlayerName;
    LoadedPlayerData.PlayerSensitivity = PlayerData->Cfg_PlayerSensitivity;
    LoadedPlayerData.FPSLimit = PlayerData->Cfg_FPSLimit;
    LastServerIP = PlayerData->Cfg_LastServerIP;
}

void UWnSGameInstance::SaveToFile() 
{
    // Save changes on shutdown
    if(!LoadedPlayerData.PlayerName.IsEmpty()) PlayerData->Cfg_PlayerName = LoadedPlayerData.PlayerName;
    if(LoadedPlayerData.PlayerSensitivity >= 0) PlayerData->Cfg_PlayerSensitivity = LoadedPlayerData.PlayerSensitivity;
    if(LoadedPlayerData.FPSLimit >= 0) PlayerData->Cfg_FPSLimit = LoadedPlayerData.FPSLimit;
    if(!LastServerIP.IsEmpty()) PlayerData->Cfg_LastServerIP = LastServerIP;

    PlayerData->SaveConfig();
}
