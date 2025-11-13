#include "PlayerHealthComponent.h"
#include "Engine/Engine.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.1f; // Reduce frequency for performance

	// Initialize default values with proper sizing
	MaxHealth = 100;
	CurrentHealth = MaxHealth;
	TotalHitsTaken = 0;
	InvulnerabilityTime = 1.0f;
	InvulnerabilityTimeRemaining = 0.0f;

	// Initialize default damage values
	DamageValues = {
		25, // Spikes
		20, // Enemy melee  
		15, // Enemy projectile
		50  // Environmental hazard
	};
}

void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize health when game starts
	ResetHealth();

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, Log, TEXT("PlayerHealthComponent initialized with %d/%d health"), CurrentHealth, MaxHealth);
#endif
}

void UPlayerHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update invulnerability timer
	if (InvulnerabilityTimeRemaining > 0.0f)
	{
		InvulnerabilityTimeRemaining = FMath::Max(0.0f, InvulnerabilityTimeRemaining - DeltaTime);
	}
	else
	{
		// Disable tick when not needed for performance
		SetComponentTickEnabled(false);
	}
}

void UPlayerHealthComponent::TakeDamage(int32 DamageAmount, EDamageType Type)
{
	// Check if damage is valid
	if (DamageAmount <= 0)
	{
		return;
	}

	// Check if player is currently invulnerable
	if (IsInvulnerable())
	{
		return;
	}

	// Apply damage
	int32 OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth);
	
	// Increment hit counter
	TotalHitsTaken++;
	
	// Set invulnerability time
	InvulnerabilityTimeRemaining = InvulnerabilityTime;
	SetComponentTickEnabled(true); // Enable tick for invulnerability timer
	
	// Trigger damage event
	OnTakeDamage.Broadcast(DamageAmount, Type);
	
	// Trigger health changed event if health actually changed
	if (OldHealth != CurrentHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
		
		// Check for death
		if (CurrentHealth <= 0)
		{
#if UE_BUILD_DEVELOPMENT
			UE_LOG(LogTemp, Warning, TEXT("Player died after %d hits"), TotalHitsTaken);
#endif
			OnPlayerDeath.Broadcast(TotalHitsTaken);
		}
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, VeryVerbose, TEXT("Player took %d damage of type %d. Health: %d/%d, Hits taken: %d"),
		DamageAmount, (int32)Type, CurrentHealth, MaxHealth, TotalHitsTaken);
#endif
}

void UPlayerHealthComponent::ResetHealth()
{
	// Reset health to maximum
	int32 OldHealth = CurrentHealth;
	CurrentHealth = MaxHealth;
	
	// Reset hit counter
	TotalHitsTaken = 0;
	
	// Reset invulnerability time
	InvulnerabilityTimeRemaining = 0.0f;
	
	// Disable tick when not needed
	SetComponentTickEnabled(false);
	
	// Broadcast health changed event if different
	if (OldHealth != CurrentHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	}
	
#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, Log, TEXT("Player health reset to %d/%d"), CurrentHealth, MaxHealth);
#endif
}