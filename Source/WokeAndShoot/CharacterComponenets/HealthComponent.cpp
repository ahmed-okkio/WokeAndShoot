// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

void UHealthComponent::ApplyDamage(float Damage) 
{
	float DamagedHealth = HealthPoints - Damage;
	HealthPoints =	FMath::Max(DamagedHealth, 0.f);
	if(HealthPoints == 0)
	{
		GetWorld()->GetTimerManager().SetTimer(KillTimerHandle, this, &UHealthComponent::KillActor, 3.f,false);
	}
}

void UHealthComponent::ApplyHeal(float Heal) 
{
	float HealedHealth = HealthPoints + Heal;
	HealthPoints =	FMath::Min(HealedHealth, MaxHealth);
}


void UHealthComponent::KillActor() 
{
	GetOwner()->Destroy();
}
