// Fill out your copyright notice in the Description page of Project Settings.

#include "../Widgets/DeathScreenWidget.h"
#include "../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "../Character/WokeAndShootCharacter.h"
#include "../GameInstance/WnSGameInstance.h"
#include "../ConfigFiles/GameConfig.h"
#include "UMG/Public/Blueprint/WidgetBlueprintLibrary.h"
#include "WokeAndShootPlayerController.h"
#include "WokeAndShoot/ServerComponents/PlayerState/MyPlayerState.h"
#include "WokeAndShoot/DevTools/MyReadWriteHelper.h"

void AWokeAndShootPlayerController::BeginPlay() 
{
    Super::BeginPlay();

    if(AWokeAndShootGameMode* Gamemode = Cast<AWokeAndShootGameMode>(GetWorld()->GetAuthGameMode()))
    {
        PlayerInformation CurrentPlayer {PlayerName};
        Gamemode->PlayersOnline++;
        Gamemode->Players.Add(GetUniqueID(),CurrentPlayer);
    }
     
    // Updating playername on the server client
    if(IsLocalPlayerController())
    {
        if(auto MyGameInstance = Cast<UWnSGameInstance>(GetGameInstance()))
        {
            if(auto MyPlayerState = GetPlayerState<AMyPlayerState>())
            {
                PlayerState->SetPlayerName(MyGameInstance->GetPlayerName());
            }
        }  
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

void AWokeAndShootPlayerController::OnRep_PlayerState() 
{
    Super::OnRep_PlayerState();

    // Sending update player name request on first receive of playerstate.
    if(IsLocalPlayerController())
    {
        if(auto MyGameInstance = Cast<UWnSGameInstance>(GetGameInstance()))
        {
            Server_ChangeName(MyGameInstance->GetPlayerName());    
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

    UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this,EscapeScreen);
    EscapeScreen->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::OpenScoreboard() 
{
    if (Scoreboard == nullptr)
    {
        Scoreboard = Cast<UUserWidget>(CreateWidget(this, ScoreboardClass));
        Scoreboard->AddToViewport();
    }

    Scoreboard->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::CloseScoreboard() 
{
    if (Scoreboard == nullptr){return;}
    Scoreboard->SetVisibility(ESlateVisibility::Hidden);
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

void AWokeAndShootPlayerController::SetLocalPlayerName(const FString& NewName) 
{
    if(IsLocalPlayerController())
    {
        // Set local player name
        PlayerName = NewName;
        // Set online player name
        PlayerState->SetPlayerName(PlayerName);
        // Setting player name playerdata container
        if(auto MyGameInstance = Cast<UWnSGameInstance>(GetGameInstance()))
        {
            MyGameInstance->SetPlayerName(PlayerName);
        }
        // Updating locally stored on disk name
        MyReadWriteHelper::SaveStringToFile(PlayerName , "/UserSettings","/Username.cfg");
    }
}

float AWokeAndShootPlayerController::GetSensitivity() const
{
    return InternalSensitivity;
}

void AWokeAndShootPlayerController::SetSensitivity(float NewSensitivity) 
{
    InternalSensitivity = NewSensitivity;
    if(auto MyGameInstance = Cast<UWnSGameInstance>(GetGameInstance()))
    {
        MyGameInstance->SetPlayerSensitvity(NewSensitivity);
    }
}

bool AWokeAndShootPlayerController::Server_ChangeName_Validate(const FString& NewName) 
{
    return true;
}

void AWokeAndShootPlayerController::Server_ChangeName_Implementation(const FString& NewName) 
{
    if(auto MyPlayerState = GetPlayerState<AMyPlayerState>())
    {
        PlayerState->SetPlayerName(NewName);
    }
}