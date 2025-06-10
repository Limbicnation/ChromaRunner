// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHealthComponent.generated.h"

// Enum for damage types
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Spikes UMETA(DisplayName = "Spikes"),
	EnemyMelee UMETA(DisplayName = "Enemy Melee"),
	EnemyProjectile UMETA(DisplayName = "Enemy Projectile"),
	EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard"),
};

// Delegate declarations for health events
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedDelegate, int32, CurrentHealth, int32, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeathDelegate, int32, TotalHitsTaken);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakeDamageDelegate, int32, DamageAmount, EDamageType, DamageType);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIDERUNNER_API UPlayerHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerHealthComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Take damage function
	UFUNCTION(BlueprintCallable, Category = "Health")
	void TakeDamage(int32 DamageAmount, EDamageType Type);
	
	// Get current health
	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetCurrentHealth() const { return CurrentHealth; }
	
	// Get max health
	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetMaxHealth() const { return MaxHealth; }
	
	// Get health as percentage (0-1)
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercentage() const { return MaxHealth > 0 ? (float)CurrentHealth / (float)MaxHealth : 0.0f; }
	
	// Get total hits taken
	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetTotalHitsTaken() const { return TotalHitsTaken; }
	
	// Check if player is invulnerable
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsInvulnerable() const { return InvulnerabilityTimeRemaining > 0.0f; }
	
	// Reset health to max
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Player's maximum health capacity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	int32 MaxHealth = 100;
	
	// Current health value
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	int32 CurrentHealth;
	
	// Counter for total hits taken
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	int32 TotalHitsTaken = 0;
	
	// Damage values by source type
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	TArray<int32> DamageValues;
	
	// Time in seconds the player remains invulnerable after taking damage
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float InvulnerabilityTime = 1.0f;
	
	// Current invulnerability time remaining
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float InvulnerabilityTimeRemaining = 0.0f;

public:
	// Delegate for when health changes
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnHealthChangedDelegate OnHealthChanged;
	
	// Delegate for when player dies
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDeathDelegate OnPlayerDeath;
	
	// Delegate for when player takes damage
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnTakeDamageDelegate OnTakeDamage;
};