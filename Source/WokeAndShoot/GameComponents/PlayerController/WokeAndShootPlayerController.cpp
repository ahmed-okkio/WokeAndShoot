// Fill out your copyright notice in the Description page of Project Settings.

#include "../Widgets/DeathScreenWidget.h"
#include "../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "../Character/WokeAndShootCharacter.h"
#include "../GameInstance/WnSGameInstance.h"
#include "../ConfigFiles/GameConfig.h"
#include "UMG/Public/Blueprint/WidgetBlueprintLibrary.h"
#include "WokeAndShoot/GameComponents/Widgets/KillFeedWidget.h"
#include "Camera/CameraComponent.h"
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
        KillFeed = Cast<UKillFeedWidget>(CreateWidget(this, KillFeedClass));
        KillFeed->AddToViewport();
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
        InputComponent->BindAction("EscapeMenu", IE_Pressed, this, &AWokeAndShootPlayerController::ShowEscapeMenu);
        InputComponent->BindAction("Scoreboard", IE_Pressed, this, &AWokeAndShootPlayerController::ShowScoreboard);
        InputComponent->BindAction("Scoreboard", IE_Released, this, &AWokeAndShootPlayerController::HideScoreboard);
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

void AWokeAndShootPlayerController::ShowEscapeMenu() 
{
    if (EscapeScreen == nullptr)
    {
        EscapeScreen = Cast<UUserWidget>(CreateWidget(this, EscapeScreenClass));
        EscapeScreen->AddToViewport(1);
    }

    UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(this,EscapeScreen);
    EscapeScreen->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::ShowScoreboard() 
{
    if (Scoreboard == nullptr)
    {
        Scoreboard = Cast<UUserWidget>(CreateWidget(this, ScoreboardClass));
        Scoreboard->AddToViewport();
    }

    Scoreboard->SetVisibility(ESlateVisibility::Visible);
}

void AWokeAndShootPlayerController::HideScoreboard() 
{
    if (Scoreboard == nullptr || GameIsOver){return;}
    Scoreboard->SetVisibility(ESlateVisibility::Hidden);
}

void AWokeAndShootPlayerController::ClientReceiveSpawn() 
{
    IsPossessing = true;

    if(IsLocalPlayerController())
    {
        ClientSetCameraFade(false);
        HideDeathScreen();
        ShowHUD();
    }
}

void AWokeAndShootPlayerController::ClientReceiveDeath() 
{
    IsPossessing = false;
    
    if(IsLocalPlayerController())
    {
        if(GameIsOver)
        {
            HideHUD();
            ClientHandlePawnDeath();
            ClientSetCameraFade(false);
        }
        else
        {
            HideHUD();
            ShowDeathScreen();
            ClientHandlePawnDeath();
        }
    }
}

void AWokeAndShootPlayerController::ClientReceiveKillInfo(const FKillInfo& NewKillInfo) 
{
    if(IsLocalPlayerController())
    {
        KillFeed->NewKillEvent(NewKillInfo.KillerName,NewKillInfo.KilledName);
    }
}

void AWokeAndShootPlayerController::ShowDeathScreen() 
{
    if (DeathScreen == nullptr)
    {  
        DeathScreen = Cast<UDeathScreenWidget>(CreateWidget(this, DeathScreenClass));

        if (DeathScreen != nullptr)
        {
            if(auto MyPlayerState = GetPlayerState<AMyPlayerState>())
            {
                DeathScreen->KillerName = MyPlayerState->LastKilledBy;
                DeathScreen->AddToViewport();
            }
        }
    }
    else
    {
        DeathScreen->SetVisibility(ESlateVisibility::Visible);
    }
}

void AWokeAndShootPlayerController::HideDeathScreen() 
{
    if(DeathScreen != nullptr)
    {
        DeathScreen->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AWokeAndShootPlayerController::ShowHUD() 
{
    if (HUD == nullptr)
    {  
        HUD = Cast<UUserWidget>(CreateWidget(this, HUDClass));
        if (HUD != nullptr)
        {
            HUD->AddToViewport();
        }
    }
    else
    {
        HUD->SetVisibility(ESlateVisibility::Visible);
    }
}

void AWokeAndShootPlayerController::HideHUD() 
{
    if (HUD != nullptr)
    {
        HUD->SetVisibility(ESlateVisibility::Hidden);
    }
}

void AWokeAndShootPlayerController::ClientHandlePawnDeath() 
{
    if(IsLocalPlayerController())
    {
        if(auto WnSCharacter = Cast<AWokeAndShootCharacter>(GetPawn()))
        {
            WnSCharacter->PawnHandleDeath();
        }
    }
}

void AWokeAndShootPlayerController::ClientEndGame() 
{
    if(IsLocalPlayerController())
    {
        GameIsOver = true;
        ClientSetCameraFade(false);
        ClientIgnoreLookInput(true);
        ClientIgnoreMoveInput(true);
        GoToEndGameView();
        HideHUD();
        FTimerHandle MyTH;
        GetWorldTimerManager().SetTimer(MyTH, this, &AWokeAndShootPlayerController::ShowScoreboard, 6.0f, false);
    }
    
}

void AWokeAndShootPlayerController::ClientRestartGame() 
{
    if(IsLocalPlayerController())
    {
        GameIsOver = false;
        ClientIgnoreLookInput(false);
        ClientIgnoreMoveInput(false);
        GoToStartGameView();
        ShowHUD();
        HideScoreboard();
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
    if(IsPossessing)
    {
        if(auto WnSCharacter = Cast<AWokeAndShootCharacter>(GetPawn()))
        {
            WnSCharacter->SetCharacterSensitivity();
        }
    }
}

void AWokeAndShootPlayerController::SetPlayerIsDead(bool IsDead) 
{
    InternalIsDead = IsDead;
}

bool AWokeAndShootPlayerController::GetPlayerIsDead() 
{
    return InternalIsDead;
}

bool AWokeAndShootPlayerController::Multi_ClientEndGame_Validate() 
{
    return true;
}

void AWokeAndShootPlayerController::Multi_ClientEndGame_Implementation() 
{
    ClientEndGame();
}

bool AWokeAndShootPlayerController::Multi_ClientRestartGame_Validate() 
{
    return true;
}

void AWokeAndShootPlayerController::Multi_ClientRestartGame_Implementation() 
{
    ClientRestartGame();
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
