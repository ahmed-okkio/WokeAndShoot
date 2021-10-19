// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "WnSGameInstance.generated.h"

class AGameConfig;

UCLASS()
class WOKEANDSHOOT_API UWnSGameInstance : public UGameInstance
{
	GENERATED_BODY()
	struct LoadedPlayerData
	{
		FString PlayerName;
		float PlayerSensitivity;
	};
	
private:
	UWnSGameInstance();

public:
	UPROPERTY()
	AGameConfig* PlayerData;

	LoadedPlayerData LoadedPlayerData;

	FString LastServerIP;

public:
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void StartGameInstance() override;

	UFUNCTION(BlueprintPure, Category="Player Information")
	FString GetPlayerName() const;
	void SetPlayerName(FString& NewPlayerName) ;
	UFUNCTION(BlueprintPure, Category="Player Information")
	float GetPlayerSensitivity() const;
	void SetPlayerSensitvity(float NewPlayerSensitivity);
	UFUNCTION(BlueprintPure, Category="Main Menu")
	FString GetSavedIP() const;
	UFUNCTION(BlueprintCallable, Category="Main Menu")
	void SetSavedIP(const FString& NewServerIP);

	UFUNCTION(BlueprintCallable, Category="Player Information")
	void LoadPlayerData();
	UFUNCTION(BlueprintCallable, Category="Player Information")
	void SaveToFile();
	
};
