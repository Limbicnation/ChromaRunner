// Fill out your copyright notice in the Description page of Project Settings.

#include "HealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "RunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "SideRunner.h" // Custom log categories

UHealthBarWidget::UHealthBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// PERFORMANCE: Disable tick by default - use event-driven updates
	bIsVariable = true;
}

void UHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Validate widget bindings
	if (!ValidateWidgetBindings())
	{
		UE_LOG(LogSideRunner, Error, TEXT("HealthBarWidget: Widget bindings validation failed!"));
		return;
	}

	// Attempt to bind to health component (with retry mechanism)
	TryBindToHealthComponent();
}

void UHealthBarWidget::NativeDestruct()
{
	// Clear retry timer if still running
	if (BindRetryTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(BindRetryTimerHandle);
	}

	// Unbind from health component to prevent dangling references
	UnbindFromHealthComponent();

	Super::NativeDestruct();
}

void UHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// PERFORMANCE: Tick disabled by default
	// If you need smooth health bar animation, enable this and implement interpolation here
}

// ============================================================================
// Health Component Event Handlers
// ============================================================================

void UHealthBarWidget::OnHealthChanged(int32 NewHealth, int32 NewMaxHealth)
{
	// Update cached values
	CurrentHealth = static_cast<float>(NewHealth);
	MaxHealth = static_cast<float>(NewMaxHealth);

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, VeryVerbose, TEXT("HealthBarWidget: Health changed to %d / %d (%.1f%%)"),
		NewHealth, NewMaxHealth, GetHealthPercent() * 100.0f);
#endif

	// Update visual state
	UpdateHealthBar();
}

void UHealthBarWidget::OnTakeDamage(int32 DamageAmount, EDamageType DamageType)
{
	// Fetch hit count from health component (single source of truth)
	if (HealthComponent)
	{
		HitCount = HealthComponent->GetTotalHitsTaken();
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, VeryVerbose, TEXT("HealthBarWidget: Took %d damage (Type: %d), Total hits: %d"),
		DamageAmount, static_cast<int32>(DamageType), HitCount);
#endif

	// Update hit counter display
	UpdateHitCounter();

	// Optional: Add damage flash animation here
	// TODO: Implement damage flash effect
}

void UHealthBarWidget::OnPlayerDeath(int32 TotalHitsTaken)
{
	HitCount = TotalHitsTaken;

	// NOTE: Death logging handled authoritatively by PlayerHealthComponent

	// Update final state
	UpdateHealthBar();
	UpdateHitCounter();

	// Optional: Add death animation/fade out here
}

// ============================================================================
// Visual Update Functions
// ============================================================================

void UHealthBarWidget::UpdateHealthBar()
{
	if (!HealthProgressBar)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: HealthProgressBar is null!"));
		return;
	}

	// Calculate health percentage
	const float HealthPercent = GetHealthPercent();

	// Update progress bar fill
	HealthProgressBar->SetPercent(HealthPercent);

	// Update color based on health percentage
	const FLinearColor BarColor = GetHealthColor();
	HealthProgressBar->SetFillColorAndOpacity(BarColor);

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, Verbose, TEXT("HealthBarWidget: Updated bar to %.1f%% with color (%.2f, %.2f, %.2f)"),
		HealthPercent * 100.0f, BarColor.R, BarColor.G, BarColor.B);
#endif
}

void UHealthBarWidget::UpdateHitCounter()
{
	if (!HitCounterText)
	{
		// Hit counter is optional, don't log warning
		return;
	}

	// Format hit counter text using configurable format
	const FText FormattedText = FText::Format(
		HitCounterFormat,
		FText::AsNumber(HitCount)
	);

	HitCounterText->SetText(FormattedText);
}

FLinearColor UHealthBarWidget::GetHealthColor() const
{
	const float HealthPercent = GetHealthPercent();

	if (!bUseSmoothColorTransition)
	{
		// Simple threshold-based color selection
		if (HealthPercent >= 0.66f)
		{
			return HealthyColor; // Green
		}
		else if (HealthPercent >= 0.33f)
		{
			return CautionColor; // Yellow
		}
		else
		{
			return CriticalColor; // Red
		}
	}
	else
	{
		// PERFORMANCE: Smooth color interpolation
		if (HealthPercent >= 0.66f)
		{
			// Interpolate from yellow to green (66% - 100%)
			const float Alpha = (HealthPercent - 0.66f) / 0.34f; // Normalize to 0-1
			return FLinearColor::LerpUsingHSV(CautionColor, HealthyColor, Alpha);
		}
		else if (HealthPercent >= 0.33f)
		{
			// Interpolate from red to yellow (33% - 66%)
			const float Alpha = (HealthPercent - 0.33f) / 0.33f; // Normalize to 0-1
			return FLinearColor::LerpUsingHSV(CriticalColor, CautionColor, Alpha);
		}
		else
		{
			// Critical health - pure red
			return CriticalColor;
		}
	}
}

float UHealthBarWidget::GetHealthPercent() const
{
	if (MaxHealth <= 0.0f)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
}

// ============================================================================
// Health Component Binding
// ============================================================================

void UHealthBarWidget::TryBindToHealthComponent()
{
	// Attempt binding
	if (BindToHealthComponent())
	{
		// Success - clear retry timer if it was set
		if (BindRetryTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		}

#if UE_BUILD_DEVELOPMENT
		UE_LOG(LogSideRunner, Log, TEXT("HealthBarWidget: Successfully bound to health component"));
#endif

		// Initialize visual state
		UpdateHealthBar();
		UpdateHitCounter();
	}
	else
	{
		// Failed - set up retry timer if not already running
		if (!BindRetryTimerHandle.IsValid())
		{
			UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: Failed to bind to health component - retrying every 0.1s"));

			// Retry every 0.1 seconds until successful
			GetWorld()->GetTimerManager().SetTimer(
				BindRetryTimerHandle,
				this,
				&UHealthBarWidget::TryBindToHealthComponent,
				0.1f,
				true  // Loop
			);
		}
	}
}

bool UHealthBarWidget::BindToHealthComponent()
{
	// Try to get the owning player's character
	APlayerController* PlayerController = GetOwningPlayer();
	if (!PlayerController)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: No owning player controller"));
		return false;
	}

	// Get the controlled pawn
	APawn* ControlledPawn = PlayerController->GetPawn();
	if (!ControlledPawn)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: Player controller has no pawn"));
		return false;
	}

	// Cast to RunnerCharacter
	OwningCharacter = Cast<ARunnerCharacter>(ControlledPawn);
	if (!OwningCharacter)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: Pawn is not a RunnerCharacter"));
		return false;
	}

	// Get the health component
	HealthComponent = OwningCharacter->HealthComponent;
	if (!HealthComponent)
	{
		UE_LOG(LogSideRunner, Error, TEXT("HealthBarWidget: RunnerCharacter has no HealthComponent!"));
		return false;
	}

	// Bind to health component delegates
	HealthComponent->OnHealthChanged.AddDynamic(this, &UHealthBarWidget::OnHealthChanged);
	HealthComponent->OnTakeDamage.AddDynamic(this, &UHealthBarWidget::OnTakeDamage);
	HealthComponent->OnPlayerDeath.AddDynamic(this, &UHealthBarWidget::OnPlayerDeath);

	// Initialize state from current health values
	CurrentHealth = static_cast<float>(HealthComponent->GetCurrentHealth());
	MaxHealth = static_cast<float>(HealthComponent->GetMaxHealth());
	HitCount = HealthComponent->GetTotalHitsTaken();

	return true;
}

void UHealthBarWidget::UnbindFromHealthComponent()
{
	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.RemoveDynamic(this, &UHealthBarWidget::OnHealthChanged);
		HealthComponent->OnTakeDamage.RemoveDynamic(this, &UHealthBarWidget::OnTakeDamage);
		HealthComponent->OnPlayerDeath.RemoveDynamic(this, &UHealthBarWidget::OnPlayerDeath);

#if UE_BUILD_DEVELOPMENT
		UE_LOG(LogSideRunner, Log, TEXT("HealthBarWidget: Unbound from health component"));
#endif
	}

	HealthComponent = nullptr;
	OwningCharacter = nullptr;
}

bool UHealthBarWidget::ValidateWidgetBindings() const
{
	bool bIsValid = true;

	if (!HealthProgressBar)
	{
		UE_LOG(LogSideRunner, Error, TEXT("HealthBarWidget: HealthProgressBar is not bound! "
			"Make sure you have a ProgressBar widget named 'HealthProgressBar' in your UMG Designer."));
		bIsValid = false;
	}

	if (!HitCounterText)
	{
		UE_LOG(LogSideRunner, Warning, TEXT("HealthBarWidget: HitCounterText is not bound (optional). "
			"Add a TextBlock widget named 'HitCounterText' to display hit counter."));
		// Not an error - hit counter is optional
	}

	return bIsValid;
}
