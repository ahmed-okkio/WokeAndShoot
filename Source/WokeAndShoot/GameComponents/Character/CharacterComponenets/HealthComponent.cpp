// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "../../../ServerComponents/GamemodeClasses/Base/WokeAndShootGameMode.h"
#include "../../Character/WokeAndShootCharacter.h"
#include "../../PlayerController/WokeAndShootPlayerController.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	HealthPoints = MaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHealthComponent::ApplyDamage(float Damage, AController* Killer) 
{
	float DamagedHealth = HealthPoints - Damage;
	HealthPoints =	FMath::Max(DamagedHealth, 0.f);
	if(HealthPoints == 0)
	{
		if(auto OwnerCharacter = Cast<AWokeAndShootCharacter>(GetOwner()))
			OwnerCharacter->TP_Body->SetUsingAbsoluteRotation(true);	
		Server_KillActor(Killer);
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

	
	if(AWokeAndShootGameMode* Gamemode = GetWorld()->GetAuthGameMode<AWokeAndShootGameMode>())
	{
		Gamemode->PawnKilled(PlayerController, Killer);
	}	
}
