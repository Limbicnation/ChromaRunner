#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerHealth.generated.h"

// ─── Damage Type Enum ──────────────────────────────────────────────────────────

/** Damage categories used throughout the game. Add new types as needed. */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
	Spikes              UMETA(DisplayName = "Spikes"),
	EnemyMelee          UMETA(DisplayName = "Enemy Melee"),
	EnemyProjectile     UMETA(DisplayName = "Enemy Projectile"),
	EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard")
};

// ─── Health State Enum ─────────────────────────────────────────────────────────

/** Tracks the high-level state of the player's health. */
UENUM(BlueprintType)
enum class EPlayerHealthState : uint8
{
	Alive         UMETA(DisplayName = "Alive"),
	Invulnerable  UMETA(DisplayName = "Invulnerable"),
	Dead          UMETA(DisplayName = "Dead")
};

// ─── Configuration Struct ──────────────────────────────────────────────────────

/** Data-driven configuration for UPlayerHealth. Editable per-instance in the editor. */
USTRUCT(BlueprintType)
struct FPlayerHealthConfig
{
	GENERATED_BODY()

	/** Maximum hit points. Must be > 0. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "1.0", UIMin = "1.0"))
	float MaxHealth = 3.0f;

	/** Duration of invulnerability window after taking damage (seconds). 0 = no invulnerability. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float InvulnerabilityDuration = 1.5f;

	/** If true, health is set to MaxHealth on BeginPlay. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	bool bStartAtFullHealth = true;
};

// ─── Delegate Declarations ─────────────────────────────────────────────────────

/** Fires whenever CurrentHealth or MaxHealth changes. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);

/** Fires when MaxHealth is changed programmatically. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChanged, float, CurrentHealth, float, MaxHealth);

/** Fires once when health reaches 0 or below. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);

/** Legacy delegate — fires on each damage tick with int32 amount and damage type. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTakeDamage, int32, DamageAmount, EDamageType, DamageType);

/** Legacy delegate — fires on death with total hit count. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDeath, int32, TotalHitsTaken);

/** Non-dynamic delegate — fires once after InitHealth completes. */
DECLARE_MULTICAST_DELEGATE(FOnHealthInitialized);

/** Fires when invulnerability begins. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerabilityStarted);

/** Fires when invulnerability ends. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInvulnerabilityEnded);

// ─── Forward Declarations ──────────────────────────────────────────────────────

class AActor;

// ─── UPlayerHealth ─────────────────────────────────────────────────────────────

/**
 * Modular, data-driven health component for any actor.
 *
 * Architecture:
 *   - Configured via FPlayerHealthConfig (exposed in editor).
 *   - Uses UE delegates for decoupled Observer-pattern communication.
 *   - No direct references to UI — UI binds to delegates externally.
 *   - Invulnerability timer prevents duplicate damage within the same window.
 *
 * Usage:
 *   1. Add UPlayerHealth as a component to your actor (e.g. RunnerCharacter).
 *   2. Configure FPlayerHealthConfig in the Details panel.
 *   3. Bind to FOnHealthChanged / FOnDeath from UI or game logic.
 *   4. Call TakeDamage() from enemies, spikes, etc.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIDERUNNER_API UPlayerHealth : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerHealth();

	// ── Configuration ───────────────────────────────────────────────────────

	/** Data-driven config exposed in the editor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
	FPlayerHealthConfig Config;

	// ── Delegates (Observer Pattern) ────────────────────────────────────────

	/** Fires whenever health changes. UI should bind to this. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnHealthChanged OnHealthChanged;

	/** Fires when MaxHealth is changed. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnMaxHealthChanged OnMaxHealthChanged;

	/** Fires once when the actor dies. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnDeath OnDeath;

	/** Legacy — fires on each damage tick. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events|Legacy")
	FOnTakeDamage OnTakeDamage;

	/** Legacy — fires on death with total hits. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events|Legacy")
	FOnPlayerDeath OnPlayerDeath;

	/** Non-dynamic — fires after initialization. */
	FOnHealthInitialized OnHealthInitialized;

	/** Fires when invulnerability begins. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnInvulnerabilityStarted OnInvulnerabilityStarted;

	/** Fires when invulnerability ends. */
	UPROPERTY(BlueprintAssignable, Category = "Health|Events")
	FOnInvulnerabilityEnded OnInvulnerabilityEnded;

	// ── Core API ────────────────────────────────────────────────────────────

	/**
	 * Apply damage to the actor.
	 * @param DamageAmount  Raw damage value (clamped to current health).
	 * @param DamageType    Category of damage for analytics / gameplay logic.
	 * @return Actual damage dealt (0 if invulnerable, dead, or uninitialized).
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float TakeDamage(float DamageAmount, EDamageType DamageType);

	/**
	 * Heal the actor.
	 * @param Amount  HP to restore (clamped to MaxHealth).
	 * @return Actual amount healed (0 if dead or uninitialized).
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	float Heal(float Amount);

	/**
	 * Manually set current health (clamped to [0, MaxHealth]).
	 * Triggers death if health reaches 0.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealth(float NewHealth);

	/**
	 * Change maximum health. Current health is clamped to the new max.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(float NewMaxHealth);

	/** Returns health as a normalized 0.0–1.0 ratio. */
	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const;

	/** True if health has been initialized via InitHealth() or BeginPlay auto-init. */
	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsFullyInitialized() const;

	/** Re-initialize health to Config defaults. Safe to call multiple times. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitHealth();

	/** Reset health, death state, hit counter, and invulnerability. */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void ResetHealth();

	// ── Invulnerability ─────────────────────────────────────────────────────

	/** Start an invulnerability window for the given duration (seconds). */
	UFUNCTION(BlueprintCallable, Category = "Health|Invulnerability")
	void TriggerInvulnerability(float Duration);

	/** True if currently invulnerable. */
	UFUNCTION(BlueprintPure, Category = "Health|Invulnerability")
	bool IsInvulnerable() const;

	// ── Backward-Compatibility Shims ────────────────────────────────────────
	// These map the old UPlayerHealthComponent API to the new class so that
	// existing Blueprint nodes and C++ callers continue to work.

	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetTotalHitsTaken() const { return TotalHitsTaken; }

	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetCurrentHealth() const { return FMath::RoundHalfFromZero(CurrentHealth); }

	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetMaxHealthInt() const { return FMath::RoundHalfFromZero(Config.MaxHealth); }

	UFUNCTION(BlueprintPure, Category = "Health")
	int32 GetMaxHealth() const { return FMath::RoundHalfFromZero(Config.MaxHealth); }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Health|Invulnerability")
	bool IsInvincible() const;

	/** Legacy alias — maps to TriggerInvulnerability. */
	UFUNCTION(BlueprintCallable, Category = "Health|Invulnerability")
	void SetInvulnerabilityTime(float Duration) { TriggerInvulnerability(Duration); }

	/** Direct MaxHealth accessor for callers that need the raw float. */
	float GetMaxHealthFloat() const { return Config.MaxHealth; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ── Internal State ──────────────────────────────────────────────────────

	float CurrentHealth = 0.0f;
	bool bInitialized = false;
	bool bDead = false;
	bool bInvulnerable = false;
	int32 TotalHitsTaken = 0;
	float InvulnerabilityTimer = 0.0f;

	static constexpr float INVULNERABILITY_TICK_RATE = 0.05f;

	FTimerHandle InvulnerabilityTimerHandle;

	// ── Internal Helpers ────────────────────────────────────────────────────

	void BroadcastHealthChange();
	void TickInvulnerability();
	void TriggerDeath();
};
