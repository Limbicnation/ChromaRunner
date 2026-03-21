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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLegacyOnTakeDamage, int32, DamageAmount, EDamageType, DamageType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLegacyOnPlayerDeath, int32, TotalHitsTaken);
DECLARE_MULTICAST_DELEGATE(FOnHealthInitialized);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIDERUNNER_API UPlayerHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPlayerHealthComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health",
        meta = (ClampMin = "0.0", UIMin = "0.0"))
    float CurrentHealth = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health",
        meta = (ClampMin = "1.0", UIMin = "1.0"))
    float MaxHealth = 3.0f;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnMaxHealthChanged OnMaxHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events")
    FOnDeath OnDeath;

    // Legacy delegates (backward compat) — signatures match old int32-based API
    UPROPERTY(BlueprintAssignable, Category = "Health|Events|Legacy")
    FLegacyOnTakeDamage OnTakeDamage;

    UPROPERTY(BlueprintAssignable, Category = "Health|Events|Legacy")
    FLegacyOnPlayerDeath OnPlayerDeath;

    FOnHealthInitialized OnHealthInitialized;

    UFUNCTION(BlueprintCallable, Category = "Health")
    float TakeDamage(float DamageAmount, EDamageType DamageType);

    UFUNCTION(BlueprintCallable, Category = "Health")
    float Heal(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float NewMaxHealth);

    UFUNCTION(BlueprintPure, Category = "Health")
    float GetHealthPercent() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsDead() const;

    UFUNCTION(BlueprintPure, Category = "Health")
    bool IsFullyInitialized() const;

    UFUNCTION(BlueprintCallable, Category = "Health")
    void InitHealth();

    UFUNCTION(BlueprintCallable, Category = "Health|Invincibility")
    void TriggerInvincibility(float Duration);

    // ── Backward-compat shims (maps old API to new) ─────────────────────────
    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetTotalHitsTaken() const { return TotalHitsTaken; }

    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetCurrentHealth() const { return FMath::RoundHalfFromZero(CurrentHealth); }

    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetMaxHealthInt() const { return FMath::RoundHalfFromZero(MaxHealth); }

    UFUNCTION(BlueprintPure, Category = "Health")
    int32 GetMaxHealth() const { return FMath::RoundHalfFromZero(MaxHealth); }

    UFUNCTION(BlueprintCallable, Category = "Health")
    void ResetHealth();

    UFUNCTION(BlueprintCallable, Category = "Health|Invincibility")
    void SetInvulnerabilityTime(float Duration) { TriggerInvincibility(Duration); }

    UFUNCTION(BlueprintPure, Category = "Health|Invincibility")
    bool IsInvulnerable() const { return bInvincible; }

    // ── New API ───────────────────────────────────────────────────────────
    UFUNCTION(BlueprintPure, Category = "Health|Invincibility")
    bool IsInvincible() const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    bool bInitialized = false;
    bool bDead = false;
    bool bInvincible = false;
    int32 TotalHitsTaken = 0;
    float InvincibilityTimer = 0.0f;

    static constexpr float INVINCIBILITY_TICK_RATE = 0.05f;

    FTimerHandle InvincibilityTimerHandle;

    UFUNCTION()
    void BroadcastHealthChange();

    UFUNCTION()
    void TickInvincibility();

    UFUNCTION()
    void TriggerDeath();
};
