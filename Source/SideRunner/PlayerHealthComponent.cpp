#include "PlayerHealthComponent.h"

// Sets default values for this component's properties
UPlayerHealthComponent::UPlayerHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame
	PrimaryComponentTick.bCanEverTick = true;

	// Initialize default damage values
	DamageValues = {
		10, // Spikes
		15, // Enemy melee
		25, // Enemy projectile
		50  // Environmental hazard
	};
}

// Called when the game starts
void UPlayerHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize health when game starts
	ResetHealth();
}

// Called every frame
void UPlayerHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update invulnerability timer
	if (InvulnerabilityTimeRemaining > 0.0f)
	{
		InvulnerabilityTimeRemaining -= DeltaTime;
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
	
	// Trigger damage event
	OnTakeDamage.Broadcast(DamageAmount, Type);
	
	// Trigger health changed event if health actually changed
	if (OldHealth != CurrentHealth)
	{
		OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
		
		// Check for death
		if (CurrentHealth <= 0)
		{
			OnPlayerDeath.Broadcast(TotalHitsTaken);
		}
	}

	// Log damage information
	UE_LOG(LogTemp, Log, TEXT("Player took %d damage of type %s. Health: %d/%d, Hits taken: %d"), 
		DamageAmount, *UEnum::GetValueAsString(Type), CurrentHealth, MaxHealth, TotalHitsTaken);
}

void UPlayerHealthComponent::ResetHealth()
{
	// Reset health to maximum
	CurrentHealth = MaxHealth;
	
	// Reset hit counter
	TotalHitsTaken = 0;
	
	// Reset invulnerability time
	InvulnerabilityTimeRemaining = 0.0f;
	
	// Broadcast health changed event
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
	
	// Log health reset
	UE_LOG(LogTemp, Log, TEXT("Player health reset to %d/%d"), CurrentHealth, MaxHealth);
}