// Fill out your copyright notice in the Description page of Project Settings.

#include "WokeAndShootPlayerController.h"
#include "../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "WokeAndShoot/ServerComponents/PlayerState/MyPlayerState.h"
#include "../Widgets/DeathScreenWidget.h"
#include "../Character/WokeAndShootCharacter.h"
#include "WokeAndShoot/DevTools/MyReadWriteHelper.h"
#include "UMG/Public/Blueprint/WidgetBlueprintLibrary.h"

void AWokeAndShootPlayerController::BeginPlay() 
{
    Super::BeginPlay();
    AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode());
    if(Gamemode != nullptr)
    {
        PlayerInformation CurrentPlayer {PlayerName};
        Gamemode->PlayersOnline++;
        Gamemode->Players.Add(GetUniqueID(),CurrentPlayer);
    }
}

AWokeAndShootPlayerController::AWokeAndShootPlayerController() 
{
    PlayerName = MyReadWriteHelper::LoadFileToString("Username.cfg","UserSettings");
}

void AWokeAndShootPlayerController::OpenEscapeMenu() 
{
    if(!IsLocalPlayerController()){return;}
    if (EscapeScreen == nullptr)
    {
        EscapeScreen = Cast<UUserWidget>(CreateWidget(this, EscapeScreenClass));
    }

    if(!EscapeScreen->IsInViewport())
    {
        UWidgetBlueprintLibrary::SetInputMode_UIOnly(this,EscapeScreen,true);
        EscapeScreen->AddToViewport();
        bShowMouseCursor = true;
    }  
}

void AWokeAndShootPlayerController::SetupInputComponent() 
{
    Super::SetupInputComponent();
    InputComponent->BindAction("EscapeMenu", IE_Pressed, this, &AWokeAndShootPlayerController::OpenEscapeMenu);
}

void AWokeAndShootPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason) 
{
    Super::EndPlay(EndPlayReason);

    if(AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode()))
    {
        Gamemode->PlayersOnline--;
        Gamemode->Players.Remove(GetUniqueID());
    }
}

void AWokeAndShootPlayerController::DisplayDeadWidget(FString KilledBy) 
{
    if(!IsLocalPlayerController()){return;}
    DeathScreen = Cast<UDeathScreenWidget>(CreateWidget(this, DeathScreenClass));
    if(DeathScreen != nullptr)
    {
        DeathScreen->KillerName = KilledBy;
        DeathScreen->AddToViewport();
    }  
}

void AWokeAndShootPlayerController::ClearDeadWidget() 
{
    if(!IsLocalPlayerController()){return;}
    if(DeathScreen != nullptr)
    {
        DeathScreen->RemoveFromViewport();
    }
}