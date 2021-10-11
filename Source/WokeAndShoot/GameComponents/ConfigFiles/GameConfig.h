// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameConfig.generated.h"

UCLASS(Config=WnSPlayer)
class WOKEANDSHOOT_API AGameConfig : public AActor
{
	GENERATED_BODY()

public:	
	UPROPERTY(Config)
	FString Cfg_PlayerName;
	UPROPERTY(Config)
	float Cfg_PlayerSensitivity;
	UPROPERTY(Config)
	FString Cfg_LastServerIP
public:	
	AGameConfig();
};
