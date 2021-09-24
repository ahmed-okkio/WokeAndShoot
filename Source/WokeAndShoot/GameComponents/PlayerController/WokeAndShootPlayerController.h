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
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> HUDClass;
	UPROPERTY()
	UUserWidget* HUD = nullptr;

	FString PlayerName;
	float InternalSensitivity;
	
	bool IsPossessing = false;

protected:
	virtual void BeginPlay()override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupInputComponent() override;
	virtual void OnRep_PlayerState() override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	AWokeAndShootPlayerController();

	void OpenEscapeMenu();
	void OpenScoreboard();
	void CloseScoreboard();

public:
	void LocalOnPossess();
	void LocalOnUnPossess();
	void DisplayDeadWidget(FString KilledBy);
	void ClearDeadWidget();
	
	UFUNCTION(BlueprintPure, Category="Player Information")
	FString GetLocalPlayerName() const;
	UFUNCTION(BlueprintCallable, Category="Player Information")
	void SetLocalPlayerName(const FString& NewName);

	UFUNCTION(BlueprintPure, Category="Player Settings")
	float GetSensitivity() const;
	UFUNCTION(BlueprintCallable, Category="Player Settings")
	void SetSensitivity(float NewSensitivity);

private:
	// Network
	UFUNCTION(Server,Reliable,WithValidation)
	void Server_ChangeName(const FString& NewName);
	bool Server_ChangeName_Validate(const FString& NewName);
	void Server_ChangeName_Implementation(const FString& NewName);
};
