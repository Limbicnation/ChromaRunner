#include "PlayerHealth.h"
#include "SideRunner.h"
#include "TimerManager.h"
#include "Engine/World.h"

UPlayerHealth::UPlayerHealth()
{
	PrimaryComponentTick.bCanEverTick = false;
	bInitialized = false;
}

void UPlayerHealth::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogSideRunner, Log, TEXT("[Health] UPlayerHealth spawned on %s"), *GetOwner()->GetName());
	InitHealth();
}

void UPlayerHealth::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
	}
	Super::EndPlay(EndPlayReason);
}

// ─── Initialization ────────────────────────────────────────────────────────────

void UPlayerHealth::InitHealth()
{
	if (bInitialized)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("[Health] InitHealth() called twice on %s"), *GetOwner()->GetName());
		return;
	}

	if (Config.MaxHealth <= 0.0f)
	{
		UE_LOG(LogSideRunner, Error, TEXT("[Health] MaxHealth must be > 0 on %s. Clamping to 1."), *GetOwner()->GetName());
		Config.MaxHealth = 1.0f;
	}

	if (Config.bStartAtFullHealth)
	{
		CurrentHealth = Config.MaxHealth;
	}
	else
	{
		CurrentHealth = FMath::Clamp(CurrentHealth, 0.0f, Config.MaxHealth);
	}

	bInitialized = true;
	bDead = false;

	UE_LOG(LogSideRunner, Log,
		TEXT("[Health] %s initialized — Health: %.1f / %.1f"),
		*GetOwner()->GetName(), CurrentHealth, Config.MaxHealth);

	OnHealthInitialized.Broadcast();
}

bool UPlayerHealth::IsFullyInitialized() const
{
	return bInitialized;
}

// ─── Core Damage / Heal ────────────────────────────────────────────────────────

float UPlayerHealth::TakeDamage(float DamageAmount, EDamageType DamageType)
{
	if (!bInitialized)
	{
		UE_LOG(LogSideRunner, Error, TEXT("[Health] TakeDamage() called before InitHealth() on %s"), *GetOwner()->GetName());
		return 0.0f;
	}

	if (bInvulnerable || IsDead())
	{
		UE_LOG(LogSideRunner, Verbose,
			TEXT("[Health] TakeDamage(%.1f) ignored on %s — Invulnerable=%d, Dead=%d"),
			DamageAmount, *GetOwner()->GetName(), bInvulnerable, IsDead());
		return 0.0f;
	}

	const float ActualDamage = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, Config.MaxHealth);

	TotalHitsTaken++;
	OnTakeDamage.Broadcast(FMath::RoundHalfFromZero(ActualDamage), DamageType);

	UE_LOG(LogSideRunner, Log,
		TEXT("[Health] %s took %.1f damage → Health: %.1f / %.1f"),
		*GetOwner()->GetName(), ActualDamage, CurrentHealth, Config.MaxHealth);

	BroadcastHealthChange();

	if (CurrentHealth <= 0.0f)
	{
		TriggerDeath();
	}
	else if (Config.InvulnerabilityDuration > 0.0f)
	{
		TriggerInvulnerability(Config.InvulnerabilityDuration);
	}

	return ActualDamage;
}

float UPlayerHealth::Heal(float Amount)
{
	if (!bInitialized || IsDead()) return 0.0f;
	if (Amount <= 0.0f) return 0.0f;

	const float OldHealth = CurrentHealth;
	CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.0f, Config.MaxHealth);
	const float AmountHealed = CurrentHealth - OldHealth;

	if (AmountHealed > 0.0f)
	{
		BroadcastHealthChange();
	}

	return AmountHealed;
}

void UPlayerHealth::SetHealth(float NewHealth)
{
	if (!bInitialized) return;
	CurrentHealth = FMath::Clamp(NewHealth, 0.0f, Config.MaxHealth);
	BroadcastHealthChange();
	if (CurrentHealth <= 0.0f) TriggerDeath();
}

void UPlayerHealth::SetMaxHealth(float NewMaxHealth)
{
	if (NewMaxHealth <= 0.0f) return;

	const float OldMaxHealth = Config.MaxHealth;
	Config.MaxHealth = NewMaxHealth;

	if (CurrentHealth > Config.MaxHealth)
	{
		CurrentHealth = Config.MaxHealth;
	}

	UE_LOG(LogSideRunner, Log,
		TEXT("[Health] MaxHealth changed: %.1f → %.1f"),
		OldMaxHealth, Config.MaxHealth);

	BroadcastHealthChange();
	OnMaxHealthChanged.Broadcast(CurrentHealth, Config.MaxHealth);
}

void UPlayerHealth::ResetHealth()
{
	CurrentHealth = Config.MaxHealth;
	bDead = false;
	bInitialized = true;
	TotalHitsTaken = 0;

	bInvulnerable = false;
	InvulnerabilityTimer = 0.0f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
	}

	OnHealthInitialized.Broadcast();
	BroadcastHealthChange();
}

// ─── Queries ───────────────────────────────────────────────────────────────────

float UPlayerHealth::GetHealthPercent() const
{
	if (Config.MaxHealth <= 0.0f) return 0.0f;
	return CurrentHealth / Config.MaxHealth;
}

bool UPlayerHealth::IsDead() const
{
	return bInitialized && bDead;
}

bool UPlayerHealth::IsInvulnerable() const
{
	return bInvulnerable;
}

bool UPlayerHealth::IsInvincible() const
{
	return bInvulnerable;
}

// ─── Invulnerability ───────────────────────────────────────────────────────────

void UPlayerHealth::TriggerInvulnerability(float Duration)
{
	if (Duration <= 0.0f || IsDead()) return;

	UWorld* World = GetWorld();
	if (!World) return;

	bInvulnerable = true;
	InvulnerabilityTimer = Duration;

	UE_LOG(LogSideRunner, Log,
		TEXT("[Health] Invulnerability triggered on %s for %.1fs"),
		*GetOwner()->GetName(), Duration);

	OnInvulnerabilityStarted.Broadcast();

	FTimerManager& TM = World->GetTimerManager();
	TM.ClearTimer(InvulnerabilityTimerHandle);
	TM.SetTimer(
		InvulnerabilityTimerHandle,
		this,
		&UPlayerHealth::TickInvulnerability,
		INVULNERABILITY_TICK_RATE,
		true
	);
}

void UPlayerHealth::TickInvulnerability()
{
	if (!bInvulnerable) return;

	InvulnerabilityTimer -= INVULNERABILITY_TICK_RATE;
	if (InvulnerabilityTimer <= 0.0f)
	{
		bInvulnerable = false;
		InvulnerabilityTimer = 0.0f;

		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(InvulnerabilityTimerHandle);
		}

		OnInvulnerabilityEnded.Broadcast();

		UE_LOG(LogSideRunner, Log,
			TEXT("[Health] Invulnerability ended on %s"),
			*GetOwner()->GetName());
	}
}

// ─── Internal Helpers ──────────────────────────────────────────────────────────

void UPlayerHealth::BroadcastHealthChange()
{
	OnHealthChanged.Broadcast(CurrentHealth, Config.MaxHealth);
}

void UPlayerHealth::TriggerDeath()
{
	if (bDead) return;

	bDead = true;
	UE_LOG(LogSideRunner, Log, TEXT("[Health] %s has died."), *GetOwner()->GetName());
	OnPlayerDeath.Broadcast(TotalHitsTaken);
	OnDeath.Broadcast();
}
