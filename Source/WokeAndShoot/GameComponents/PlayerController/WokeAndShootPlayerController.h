// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WokeAndShootPlayerController.generated.h"


UCLASS()
class WOKEANDSHOOT_API AWokeAndShootPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UDeathScreenWidget> DeathScreenClass;
	UDeathScreenWidget* DeathScreen = nullptr;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> EscapeScreenClass;
	UPROPERTY()
	UUserWidget* EscapeScreen = nullptr;
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> ScoreboardClass;
	UPROPERTY()
	UUserWidget* Scoreboard = nullptr;

	FString PlayerName = TEXT("Player");
	FString UserSettingsPath = TEXT("/UserSettings/Username.cfg");
private:
	AWokeAndShootPlayerController();
	void OpenEscapeMenu();
	void OpenScoreboard();
	void CloseScoreboard();

protected:
	virtual void BeginPlay()override;
	virtual void SetupInputComponent() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void DisplayDeadWidget(FString KilledBy);
	void ClearDeadWidget();
	
	UFUNCTION(BlueprintPure, Category="Player Information")
	FString GetLocalPlayerName() const;
	UFUNCTION(BlueprintCallable, Category="Player Information")
	void SetPlayerName(const FString& NewName);
};
