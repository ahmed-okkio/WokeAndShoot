// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "WokeAndShoot/WokeAndShootGameMode.h"
#include "WokeAndShoot/WokeAndShootPlayerController.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	HealthPoints = MaxHealth;
	// ...
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::ApplyDamage(float Damage, AController* Killer) 
{
	float DamagedHealth = HealthPoints - Damage;
	HealthPoints =	FMath::Max(DamagedHealth, 0.f);
	if(HealthPoints == 0)
	{
		// if(GetOwnerRole() == ROLE_AutonomousProxy)
		// {

		// 	GLog->Log("AUTO PROXY");
		// }
		// if(GetOwnerRole() == ROLE_Authority)
		// {

		// 	GLog->Log("AUTHORITY");
		// }
		if(GetOwnerRole() == ROLE_SimulatedProxy)
		{
			
		}
		if(Killer)
		{
			// GLog->Log(Killer->GetName());
		}
		Server_KillActor(Killer);
		Client_KillActor(Killer);
	}
}

void UHealthComponent::ApplyHeal(float Heal) 
{
	float HealedHealth = HealthPoints + Heal;
	HealthPoints =	FMath::Min(HealedHealth, MaxHealth);
}


void UHealthComponent::Server_KillActor(AController* Killer) 
{
	if(GetOwnerRole() != ROLE_Authority){return;}
	APawn* PlayerPawn =  Cast<APawn>(GetOwner());
	if(PlayerPawn == nullptr){return;}
	AController* PlayerController = PlayerPawn->GetController();
	if(PlayerController == nullptr){return;}
	AWokeAndShootGameMode* Gamemode = GetWorld()->GetAuthGameMode<AWokeAndShootGameMode>();
	if(Gamemode != nullptr)
	{
		Gamemode->PawnKilled(PlayerController, Killer);
	}	
}

void UHealthComponent::Client_KillActor(AController* Killer) 
{
	AWokeAndShootPlayerController* KilledPlayerController = Cast<AWokeAndShootPlayerController>(GetOwner()->GetInstigatorController());
	if(KilledPlayerController == nullptr){return;}
	if(KilledPlayerController->IsLocalController())
	{
		KilledPlayerController->PlayerKilled();
		APawn* PlayerPawn =  Cast<APawn>(GetOwner());
		if(PlayerPawn != nullptr)
		{
			// PlayerPawn->DetachFromControllerPendingDestroy();
		}
	}
}
