#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealth.h"
#include "HealthBarUI.generated.h"

class UProgressBar;
class UTextBlock;
class ARunnerCharacter;

/**
 * Decoupled health bar widget that observes UPlayerHealth via delegates.
 *
 * Architecture:
 *   - Has NO direct knowledge of the health system's internals.
 *   - Binds to UPlayerHealth delegates in NativeConstruct.
 *   - Maintains a normalized float (0.0–1.0) for fill ratio.
 *   - Optionally displays a hit counter and color transitions.
 *   - Uses TWeakObjectPtr for safe, GC-friendly reference to the health component.
 *
 * Usage:
 *   1. Create a UMG Widget Blueprint with parent class UHealthBarUI.
 *   2. Add a ProgressBar named "HealthProgressBar".
 *   3. Add a TextBlock named "HitCounterText" (optional).
 *   4. The widget auto-binds to the player's UPlayerHealth on construct.
 */
UCLASS()
class SIDERUNNER_API UHealthBarUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UHealthBarUI(const FObjectInitializer& ObjectInitializer);

	// ── Widget Bindings (must match UMG widget names exactly) ────────────────

	/** Progress bar widget — named "HealthProgressBar" in UMG Designer. */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthProgressBar;

	/** Hit counter text widget — named "HitCounterText" in UMG Designer (optional). */
	UPROPERTY(meta = (BindWidgetOptional))
	UTextBlock* HitCounterText;

	// ── Visual Configuration ────────────────────────────────────────────────

	/** Enable smooth color transitions between health thresholds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	bool bUseSmoothColorTransition = true;

	/** Color when health is above 66%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor HealthyColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	/** Color when health is between 33–66%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor CautionColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);

	/** Color when health is below 33%. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FLinearColor CriticalColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

	/** Display format for the hit counter text. {0} is replaced with the hit count. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Bar")
	FText HitCounterFormat = FText::FromString(TEXT("Hits: {0}"));

	// ── Public Query ────────────────────────────────────────────────────────

	/** Returns the current normalized health (0.0–1.0). */
	UFUNCTION(BlueprintPure, Category = "Health Bar")
	float GetNormalizedHealth() const { return NormalizedHealth; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	// ── Internal State ──────────────────────────────────────────────────────

	float NormalizedHealth = 1.0f;
	float CurrentHealth = 0.0f;
	float MaxHealth = 0.0f;
	int32 HitCount = 0;

	/** Engine-specific smart pointer — prevents dangling if health component is GC'd. */
	TWeakObjectPtr<UPlayerHealth> ObservedHealth;

	UPROPERTY()
	ARunnerCharacter* OwningCharacter;

	FTimerHandle BindRetryTimerHandle;

	// ── Delegate Handlers ───────────────────────────────────────────────────

	UFUNCTION()
	void OnHealthChanged(float NewHealth, float NewMaxHealth);

	UFUNCTION()
	void OnTakeDamage(int32 DamageAmount, EDamageType DamageType);

	UFUNCTION()
	void OnPlayerDeath(int32 TotalHitsTaken);

	// ── Visual Update ───────────────────────────────────────────────────────

	void UpdateHealthBar();
	void UpdateHitCounter();
	FLinearColor GetHealthColor() const;

	// ── Binding Logic ───────────────────────────────────────────────────────

	bool BindToHealthComponent();
	void TryBindToHealthComponent();
	void UnbindFromHealthComponent();
	bool ValidateWidgetBindings() const;
};
