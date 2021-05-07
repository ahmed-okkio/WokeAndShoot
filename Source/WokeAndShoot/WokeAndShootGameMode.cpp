// Copyright Epic Games, Inc. All Rights Reserved.

#include "WokeAndShootGameMode.h"
#include "WokeAndShootHUD.h"
#include "WokeAndShootCharacter.h"
#include "UObject/ConstructorHelpers.h"

AWokeAndShootGameMode::AWokeAndShootGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWokeAndShootHUD::StaticClass();
}
