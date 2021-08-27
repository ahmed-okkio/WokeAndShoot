// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class WOKEANDSHOOT_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	UFUNCTION()
	void OnRep_KilledBy();
	UFUNCTION()
	void OnRep_Respawn();

public:
	UPROPERTY(ReplicatedUsing = OnRep_KilledBy )
	FString LastKilledBy = "";
	UPROPERTY(ReplicatedUsing = OnRep_Respawn )
	APawn* NewPawn = nullptr;

public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
