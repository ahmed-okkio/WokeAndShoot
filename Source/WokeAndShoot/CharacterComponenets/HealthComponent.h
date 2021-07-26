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

	

	void KillActor(AController* Killer);
public:	
	// Sets default values for this component's properties
	UHealthComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float HealthPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyDamage(float Damage, AController* Killer);

	void ApplyHeal(float Heal);


		
};
