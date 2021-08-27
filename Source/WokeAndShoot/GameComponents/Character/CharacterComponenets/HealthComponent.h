// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WOKEANDSHOOT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	void Server_KillActor(AController* Killer);


protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float HealthPoints;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;	

public:	
	UHealthComponent();
	void ApplyDamage(float Damage, AController* Killer);
	void ApplyHeal(float Heal);	
};
