// Fill out your copyright notice in the Description page of Project Settings.

#include "SideRunnerPlayerController.h"
#include "SideRunnerGameInstance.h"
#include "RunnerCharacter.h"
#include "PlayerHealthComponent.h"
#include "Engine/Engine.h"

ASideRunnerPlayerController::ASideRunnerPlayerController()
{
	// Initialize cached references
	CachedGameInstance = nullptr;
}

void ASideRunnerPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Cache game instance reference for debug commands
	CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

	if (!IsValid(CachedGameInstance))
	{
		UE_LOG(LogTemp, Error, TEXT("SideRunnerPlayerController: Failed to get SideRunnerGameInstance!"));
	}
}

// ======================================================================
// Debug Console Commands (Development/Editor builds only)
// ======================================================================

#if !UE_BUILD_SHIPPING
void ASideRunnerPlayerController::DebugTriggerGameOver()
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Triggering game over via console command"));

	if (!IsValid(CachedGameInstance))
	{
		CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());
	}

	if (IsValid(CachedGameInstance))
	{
		// Force set lives to 0 to trigger game over
		CachedGameInstance->ResetLives();  // Reset to max first

		// Then decrement all lives
		while (CachedGameInstance->GetCurrentLives() > 0)
		{
			CachedGameInstance->DecrementLives();
		}

		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Game over triggered successfully"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				TEXT("DEBUG: Game Over Triggered"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot trigger game over - GameInstance is invalid!"));
	}
}

void ASideRunnerPlayerController::DebugSetScore(int32 NewScore)
{
	if (NewScore < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Invalid score value %d - must be non-negative"), NewScore);
		return;
	}

	if (!IsValid(CachedGameInstance))
	{
		CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());
	}

	if (IsValid(CachedGameInstance))
	{
		// Access protected members through public methods would require adding a setter
		// For now, log that this needs implementation
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: DebugSetScore requires a public setter in USideRunnerGameInstance"));
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Current score: %d | Requested: %d"),
			CachedGameInstance->GetCurrentScore(), NewScore);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
				FString::Printf(TEXT("DEBUG: Score change requested (needs setter): %d"), NewScore));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot set score - GameInstance is invalid!"));
	}
}

void ASideRunnerPlayerController::DebugAddLives(int32 LivestoAdd)
{
	if (LivestoAdd <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Invalid lives value %d - must be positive"), LivestoAdd);
		return;
	}

	if (!IsValid(CachedGameInstance))
	{
		CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());
	}

	if (IsValid(CachedGameInstance))
	{
		// Call ResetLives and manually add more
		// Note: This is a workaround - ideally GameInstance would have AddLives()
		const int32 CurrentLives = CachedGameInstance->GetCurrentLives();
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Cannot directly add lives - Current: %d/%d"),
			CurrentLives, CachedGameInstance->GetMaxLives());

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
				FString::Printf(TEXT("DEBUG: Lives: %d/%d (AddLives needs implementation)"),
					CurrentLives, CachedGameInstance->GetMaxLives()));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot add lives - GameInstance is invalid!"));
	}
}

void ASideRunnerPlayerController::TeleportToDistance(float DistanceMeters)
{
	ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot teleport - PlayerCharacter not found!"));
		return;
	}

	// Convert meters to Unreal units (1 meter = 100 units)
	const float TargetX = DistanceMeters * 100.0f;

	// Get current location and update X position only
	FVector NewLocation = PlayerCharacter->GetActorLocation();
	NewLocation.X = TargetX;

	// Teleport player
	PlayerCharacter->SetActorLocation(NewLocation, false, nullptr, ETeleportType::TeleportPhysics);

	// Update game instance distance tracking
	if (!IsValid(CachedGameInstance))
	{
		CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());
	}

	if (IsValid(CachedGameInstance))
	{
		CachedGameInstance->UpdateDistanceScore(TargetX);
	}

	UE_LOG(LogTemp, Warning, TEXT("DEBUG: Teleported to %.1f meters (X=%.1f units)"), DistanceMeters, TargetX);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan,
			FString::Printf(TEXT("DEBUG: Teleported to %.1f meters"), DistanceMeters));
	}
}

void ASideRunnerPlayerController::KillPlayer()
{
	ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot kill player - PlayerCharacter not found!"));
		return;
	}

	UPlayerHealthComponent* HealthComp = PlayerCharacter->HealthComponent;
	if (IsValid(HealthComp) && HealthComp->IsFullyInitialized())
	{
		const int32 MaxHealth = HealthComp->GetMaxHealth();
		HealthComp->TakeDamage(MaxHealth * 10, EDamageType::EnvironmentalHazard);

		UE_LOG(LogTemp, Warning, TEXT("DEBUG: Player killed via console command"));

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red,
				TEXT("DEBUG: Player Killed"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("DEBUG: Cannot kill player - HealthComponent is invalid or not initialized!"));
	}
}
#endif
