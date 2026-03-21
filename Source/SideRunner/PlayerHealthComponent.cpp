#include "PlayerHealthComponent.h"
#include "SideRunner.h"
#include "SideRunner/RunnerCharacter.h"
#include "GameFramework/NavMovementComponent.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

UPlayerHealthComponent::UPlayerHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bInitialized = false;
}

void UPlayerHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogSideRunner, Log, TEXT("[Health] Component spawned on %s"), *GetOwner()->GetName());
}

void UPlayerHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Clear any active timers
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
    }
    Super::EndPlay(EndPlayReason);
}

void UPlayerHealthComponent::InitHealth()
{
    if (bInitialized)
    {
        UE_LOG(LogSideRunner, Warning, TEXT("[Health] InitHealth() called twice on %s"), *GetOwner()->GetName());
        return;
    }

    if (MaxHealth <= 0.0f)
    {
        UE_LOG(LogSideRunner, Error, TEXT("[Health] MaxHealth must be > 0 on %s. Clamping to 1."), *GetOwner()->GetName());
        MaxHealth = 1.0f;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, MaxHealth);
    bInitialized = true;

    UE_LOG(LogSideRunner, Log,
        TEXT("[Health] %s initialized — Health: %.1f / %.1f"),
        *GetOwner()->GetName(), CurrentHealth, MaxHealth);

    OnHealthInitialized.Broadcast();
}

bool UPlayerHealthComponent::IsFullyInitialized() const
{
    return bInitialized;
}

float UPlayerHealthComponent::TakeDamage(float DamageAmount, EDamageType DamageType)
{
    if (!bInitialized)
    {
        UE_LOG(LogSideRunner, Error, TEXT("[Health] TakeDamage() called before InitHealth() on %s"), *GetOwner()->GetName());
        return 0.0f;
    }

    // ── Invincibility guard ────────────────────────────────────────────────
    if (bInvincible || IsDead())
    {
        UE_LOG(LogSideRunner, Verbose,
            TEXT("[Health] TakeDamage(%.1f) ignored on %s — Invincible=%d, Dead=%d"),
            DamageAmount, *GetOwner()->GetName(), bInvincible, IsDead());
        return 0.0f;
    }

    // ── Apply damage ───────────────────────────────────────────────────────
    const float ActualDamage = FMath::Min(DamageAmount, CurrentHealth);
    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    UE_LOG(LogSideRunner, Log,
        TEXT("[Health] %s took %.1f damage → Health: %.1f / %.1f"),
        *GetOwner()->GetName(), ActualDamage, CurrentHealth, MaxHealth);

    BroadcastHealthChange();

    // ── Death check ────────────────────────────────────────────────────────
    if (CurrentHealth <= 0.0f)
    {
        TriggerDeath();
    }

    return ActualDamage;
}

float UPlayerHealthComponent::Heal(float Amount)
{
    if (!bInitialized || IsDead()) return 0.0f;
    if (Amount <= 0.0f) return 0.0f;

    const float OldHealth = CurrentHealth;
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, MaxHealth);
    const float AmountHealed = CurrentHealth - OldHealth;

    if (AmountHealed > 0.0f)
    {
        BroadcastHealthChange();
    }

    return AmountHealed;
}

void UPlayerHealthComponent::SetHealth(float NewHealth)
{
    if (!bInitialized) return;
    CurrentHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
    BroadcastHealthChange();
    if (CurrentHealth <= 0.0f) TriggerDeath();
}

void UPlayerHealthComponent::SetMaxHealth(float NewMaxHealth)
{
    if (NewMaxHealth <= 0.0f) return;

    const float OldMaxHealth = MaxHealth;
    MaxHealth = NewMaxHealth;

    // Clamp current health to new max
    if (CurrentHealth > MaxHealth)
    {
        CurrentHealth = MaxHealth;
    }

    UE_LOG(LogSideRunner, Log,
        TEXT("[Health] MaxHealth changed: %.1f → %.1f"),
        OldMaxHealth, MaxHealth);

    // ── Broadcast both delegates so HUD can update MaxHealth display ──────────
    BroadcastHealthChange();                    // fires OnHealthChanged
    OnMaxHealthChanged.Broadcast(CurrentHealth, MaxHealth); // fires OnMaxHealthChanged
}

float UPlayerHealthComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

bool UPlayerHealthComponent::IsDead() const
{
    return bInitialized && CurrentHealth <= 0.0f;
}

void UPlayerHealthComponent::BroadcastHealthChange()
{
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UPlayerHealthComponent::TriggerDeath()
{
    if (IsDead()) return; // already triggered

    UE_LOG(LogSideRunner, Log, TEXT("[Health] %s has died."), *GetOwner()->GetName());
    OnDeath.Broadcast();
}

void UPlayerHealthComponent::TriggerInvincibility(float Duration)
{
    if (Duration <= 0.0f || IsDead()) return;

    bInvincible = true;
    InvincibilityTimer = Duration;

    UE_LOG(LogSideRunner, Log,
        TEXT("[Health] Invincibility triggered on %s for %.1fs"),
        *GetOwner()->GetName(), Duration);

    // Tick invincibility countdown every INVINCIBILITY_TICK_RATE seconds
    InvincibilityTimerDelegate.BindUFunction(this, TEXT("TickInvincibility"), GetWorld()->GetTimerManager().GetTimerElapsed(InvincibilityTimerHandle));

    UWorld* World = GetWorld();
    if (!World) return;

    FTimerManager& TM = World->GetTimerManager();
    TM.ClearTimer(InvincibilityTimerHandle);
    TM.SetTimer(
        InvincibilityTimerHandle,
        this,
        &UPlayerHealthComponent::TickInvincibility,
        INVINCIBILITY_TICK_RATE,
        true
    );
}

void UPlayerHealthComponent::TickInvincibility(float DeltaTime)
{
    if (!bInvincible) return;

    InvincibilityTimer -= DeltaTime;
    if (InvincibilityTimer <= 0.0f)
    {
        bInvincible = false;
        InvincibilityTimer = 0.0f;

        UWorld* World = GetWorld();
        if (World)
        {
            World->GetTimerManager().ClearTimer(InvincibilityTimerHandle);
        }

        UE_LOG(LogSideRunner, Log,
            TEXT("[Health] Invincibility ended on %s"),
            *GetOwner()->GetName());
    }
}

bool UPlayerHealthComponent::IsInvincible() const
{
    return bInvincible;
}
