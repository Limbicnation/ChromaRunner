#pragma once

#include "Components/ActorComponent.h"
#include "Delegates/Delegate.h"
#include "PlayerHealthComponent.generated.h"

/** Damage categories used throughout the game. Add new types as needed. */
UENUM(BlueprintType)
enum class EDamageType : uint8
{
    Spikes              UMETA(DisplayName = "Spikes"),
    EnemyMelee          UMETA(DisplayName = "Enemy Melee"),
    EnemyProjectile     UMETA(DisplayName = "Enemy Projectile"),
    EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard")
};

class UTextBlock;
class UProgressBar;
class AActor;

/**
 * Delegate signatures for health component events.
 * Use these to bind HUD widgets and game logic.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMaxHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
DECLARE_MULTICAST_DELEGATE(FOnHealthInitialized);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIDERUNNER_API UPlayerHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerHealthComponent();

    /** Current health value. Clamped to [0, MaxHealth]. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float CurrentHealth = 3.0f;

    /** Maximum health. Must be > 0. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health",
        meta = (ClampMin = "1.0", UIMin = "1.0"))
    float MaxHealth = 3.0f;

    // ── Delegates ──────────────────────────────────────────────────────────────

    /** Fired whenever health changes (gain or loss). */
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthChanged OnHealthChanged;

    /** Fired when max health changes (e.g. upgrade). */
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnMaxHealthChanged OnMaxHealthChanged;

    /** Fired when health reaches zero. */
    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDeath OnDeath;

    /** Fired once health component is fully initialized and health values are valid. */
    FOnHealthInitialized OnHealthInitialized;

    // ── API ───────────────────────────────────────────────────────────────────

    /** Apply damage. Respects invincibility frames. Returns actual damage dealt. */
    UFUNCTION(BlueprintCallable, Category = "Health")
    float TakeDamage(float DamageAmount, EDamageType DamageType);

    /** Heal by Amount. Will not exceed MaxHealth. Returns actual amount healed. */
    UFUNCTION(BlueprintCallable, Category = "Health")
    float Heal(float Amount);

    /** Immediately set health to a specific value. Clamped to [0, MaxHealth]. */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    /** Change max health. If NewMaxHealth < CurrentHealth, health is clamped. */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth);

    /** Returns current health as a 0..1 fraction of MaxHealth. */
    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;

    /** Returns true if health is currently at or below zero. */
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const;

    /** Returns true once InitHealth() has been called. */
    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsFullyInitialized() const;

    /** Must be called once before use, typically from the owning Blueprint's BeginPlay. */
    UFUNCTION(BlueprintCallable, Category = "Health")
    void InitHealth();

    // ── Invincibility frames ──────────────────────────────────────────────────

    /** Trigger invincibility frames for Duration seconds. Subsequent TakeDamage calls are ignored while active. */
    UFUNCTION(BlueprintCallable, Category = "Health|Invincibility")
    void TriggerInvincibility(float Duration);

    UFUNCTION(BlueprintPure, Category = "Health|Invincibility")
    bool IsInvincible() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    // ── Internal state ───────────────────────────────────────────────────────

    /** Internal flag — InitHealth() must be called before the component is usable. */
    bool bInitialized = false;

    bool bInvincible = false;
    float InvincibilityTimer = 0.0f;

    static constexpr float INVINCIBILITY_TICK_RATE = 0.05f; // 50ms

    FTimerHandle InvincibilityTimerHandle;
    FTimerDelegate InvincibilityTimerDelegate;

    // ── Helpers ─────────────────────────────────────────────────────────────

    /** Broadcast OnHealthChanged. Called after any health change. */
    UFUNCTION()
    void BroadcastHealthChange();

    /** Tick invincibility timer. */
    UFUNCTION()
    void TickInvincibility(float DeltaTime);

    /** Internal death sequence — clamp health, fire OnDeath. */
    UFUNCTION()
    void TriggerDeath();
};
