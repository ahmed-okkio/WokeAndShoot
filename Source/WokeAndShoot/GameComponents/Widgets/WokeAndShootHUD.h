// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "WokeAndShootHUD.generated.h"

UCLASS()
class AWokeAndShootHUD : public AHUD
{
	GENERATED_BODY()

public:
	AWokeAndShootHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

