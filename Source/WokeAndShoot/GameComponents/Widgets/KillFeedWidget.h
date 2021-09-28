// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KillFeedWidget.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API UKillFeedWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "HUDUpdate")
	void NewKillEvent(const FString& KillerName, const FString& KilledName);
};
