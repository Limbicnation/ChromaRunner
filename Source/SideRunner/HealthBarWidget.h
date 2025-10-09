// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthComponent.h"
#include "HealthBarWidget.generated.h"

// Forward declarations for better compilation performance
class UProgressBar;
class UTextBlock;
class ARunnerCharacter;

/**
 * Performance-optimized C++ UMG widget for displaying player health.
 * Automatically binds to PlayerHealthComponent and updates visuals in real-time.
 *
 * Usage:
 * 1. Create UMG Widget Blueprint with parent class UHealthBarWidget
 * 2. Add ProgressBar named "HealthProgressBar"
 * 3. Add TextBlock named "HitCounterText" (optional)
 * 4. Widget auto-binds to health component on construct
 */
UCLASS()
class SIDERUNNER_API UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UHealthBarWidget(const FObjectInitializer& ObjectInitializer);

	// PERFORMANCE: Widget references (bound in UMG Designer using BindWidget meta)

	/** Progress bar widget - must be named "HealthProgressBar" in UMG Designer */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	/** Hit counter text widget - must be named "HitCounterText" in UMG Designer */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HitCounterText;

	// PERFORMANCE: Configuration properties

	/** Enable smooth color transitions (slight performance cost) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	bool bUseSmoothColorTransition = true;

	/** Color when health is above 66% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor HealthyColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f); // Green

	/** Color when health is between 33-66% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor CautionColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f); // Yellow

	/** Color when health is below 33% */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor CriticalColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f); // Red

	/** Display format for hit counter text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FText HitCounterFormat = FText::FromString(TEXT("Hits: {0}"));

protected:
	// PERFORMANCE: UWidget overrides

	/** Called when widget is constructed - auto-binds to health component */
	virtual void NativeConstruct() override;

	/** Called when widget is destroyed - unbinds from health component */
	virtual void NativeDestruct() override;

	/** Called every frame (disabled by default for performance) */
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// PERFORMANCE: Health component event handlers

	/** Callback when player health changes */
	UFUNCTION()
	void OnHealthChanged(int32 NewHealth, int32 NewMaxHealth);

	/** Callback when player takes damage */
	UFUNCTION()
	void OnTakeDamage(int32 DamageAmount, EDamageType DamageType);

	/** Callback when player dies */
	UFUNCTION()
	void OnPlayerDeath(int32 TotalHitsTaken);

private:
	// PERFORMANCE: Internal state tracking

	/** Current health value (cached for interpolation) */
	float CurrentHealth = 100.0f;

	/** Maximum health value */
	float MaxHealth = 100.0f;

	/** Total hits taken counter */
	int32 HitCount = 0;

	/** Reference to player's health component */
	UPROPERTY()
	UPlayerHealthComponent* HealthComponent;

	/** Reference to owning character (cached for performance) */
	UPROPERTY()
	ARunnerCharacter* OwningCharacter;

	// PERFORMANCE: Helper functions

	/** Updates the progress bar visual state */
	void UpdateHealthBar();

	/** Updates the hit counter text */
	void UpdateHitCounter();

	/** Calculates health color based on current health percentage */
	FLinearColor GetHealthColor() const;

	/** Gets the health percentage (0.0 - 1.0) */
	float GetHealthPercent() const;

	/** Attempts to find and bind to the player's health component */
	bool BindToHealthComponent();

	/** Unbinds from health component delegates */
	void UnbindFromHealthComponent();

	/** Validates widget bindings (logs warnings if missing) */
	bool ValidateWidgetBindings() const;
};
