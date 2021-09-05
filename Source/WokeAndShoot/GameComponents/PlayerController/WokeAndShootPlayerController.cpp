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
    
    if(AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode()))
    {
        PlayerInformation CurrentPlayer {PlayerName};
        Gamemode->PlayersOnline++;
        Gamemode->Players.Add(GetUniqueID(),CurrentPlayer);
    }
    if(IsLocalPlayerController())
    {
        PlayerName = MyReadWriteHelper::LoadFileToString(UserSettingsPath);
        if(AMyPlayerState* MyPlayerState = GetPlayerState<AMyPlayerState>())
        {
            PlayerState->SetPlayerName(PlayerName);
        }
    }
}

AWokeAndShootPlayerController::AWokeAndShootPlayerController() 
{

}

void AWokeAndShootPlayerController::OpenEscapeMenu() 
{
    if (EscapeScreen == nullptr)
    {
        EscapeScreen = Cast<UUserWidget>(CreateWidget(this, EscapeScreenClass));
        EscapeScreen->AddToViewport();
    }

    UWidgetBlueprintLibrary::SetInputMode_UIOnly(this,EscapeScreen);
    EscapeScreen->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::OpenScoreboard() 
{
    if (Scoreboard == nullptr)
    {
        Scoreboard = Cast<UUserWidget>(CreateWidget(this, ScoreboardClass));
        Scoreboard->AddToViewport();
    }

    // UWidgetBlueprintLibrary::SetInputMode_UIOnly(this,Scoreboard);
    Scoreboard->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::CloseScoreboard() 
{
     if (Scoreboard == nullptr){return;}
    Scoreboard->SetVisibility(ESlateVisibility::Hidden);
}

void AWokeAndShootPlayerController::SetupInputComponent() 
{
    Super::SetupInputComponent();

    if(IsLocalPlayerController() && GetWorld()->GetName() != TEXT("Stabilize_MainMenu"))
    {
        InputComponent->BindAction("EscapeMenu", IE_Pressed, this, &AWokeAndShootPlayerController::OpenEscapeMenu);
        InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AWokeAndShootPlayerController::OpenScoreboard);
        InputComponent->BindAction("Scoreboard", IE_Released, this, &AWokeAndShootPlayerController::CloseScoreboard);
    }

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

FString AWokeAndShootPlayerController::GetLocalPlayerName() const
{
    return PlayerName;
}

void AWokeAndShootPlayerController::SetPlayerName(const FString& NewName) 
{
    if(IsLocalPlayerController())
    {
        // Set local player name
        PlayerName = NewName;
        // Set online player name
        PlayerState->SetPlayerName(PlayerName);
        // Updating locally stored on disk name
        MyReadWriteHelper::SaveStringToFile(PlayerName , UserSettingsPath);
    }
}
