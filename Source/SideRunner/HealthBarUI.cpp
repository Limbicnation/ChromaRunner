#include "HealthBarUI.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "RunnerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "SideRunner.h"

UHealthBarUI::UHealthBarUI(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = true;
}

void UHealthBarUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (!ValidateWidgetBindings())
	{
		UE_LOG(LogSideRunner, Error, TEXT("HealthBarUI: Widget bindings validation failed!"));
		return;
	}

	TryBindToHealthComponent();
}

void UHealthBarUI::NativeDestruct()
{
	if (BindRetryTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		}
	}

	UnbindFromHealthComponent();
	Super::NativeDestruct();
}

void UHealthBarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// Reserved for smooth fill animation if desired.
}

// ─── Delegate Handlers ─────────────────────────────────────────────────────────

void UHealthBarUI::OnHealthChanged(float NewHealth, float NewMaxHealth)
{
	CurrentHealth = NewHealth;
	MaxHealth = NewMaxHealth;
	NormalizedHealth = (MaxHealth > 0.0f)
		? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f)
		: 0.0f;

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, VeryVerbose,
		TEXT("HealthBarUI: Health changed to %.1f / %.1f (%.1f%%)"),
		NewHealth, NewMaxHealth, NormalizedHealth * 100.0f);
#endif

	UpdateHealthBar();
}

void UHealthBarUI::OnTakeDamage(int32 DamageAmount, EDamageType DamageType)
{
	if (ObservedHealth.IsValid())
	{
		HitCount = ObservedHealth->GetTotalHitsTaken();
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, VeryVerbose,
		TEXT("HealthBarUI: Took %d damage (Type: %d), Total hits: %d"),
		DamageAmount, static_cast<int32>(DamageType), HitCount);
#endif

	UpdateHitCounter();
}

void UHealthBarUI::OnPlayerDeath(int32 TotalHitsTaken)
{
	HitCount = TotalHitsTaken;
	UpdateHealthBar();
	UpdateHitCounter();
}

// ─── Visual Update ─────────────────────────────────────────────────────────────

void UHealthBarUI::UpdateHealthBar()
{
	if (!HealthProgressBar) return;

	HealthProgressBar->SetPercent(NormalizedHealth);
	HealthProgressBar->SetFillColorAndOpacity(GetHealthColor());
}

void UHealthBarUI::UpdateHitCounter()
{
	if (!HitCounterText) return;

	const FText FormattedText = FText::Format(
		HitCounterFormat,
		FText::AsNumber(HitCount));
	HitCounterText->SetText(FormattedText);
}

FLinearColor UHealthBarUI::GetHealthColor() const
{
	if (!bUseSmoothColorTransition)
	{
		if (NormalizedHealth >= 0.66f) return HealthyColor;
		if (NormalizedHealth >= 0.33f) return CautionColor;
		return CriticalColor;
	}

	if (NormalizedHealth >= 0.66f)
	{
		const float Alpha = (NormalizedHealth - 0.66f) / 0.34f;
		return FLinearColor::LerpUsingHSV(CautionColor, HealthyColor, Alpha);
	}
	if (NormalizedHealth >= 0.33f)
	{
		const float Alpha = (NormalizedHealth - 0.33f) / 0.33f;
		return FLinearColor::LerpUsingHSV(CriticalColor, CautionColor, Alpha);
	}
	return CriticalColor;
}

// ─── Binding Logic ─────────────────────────────────────────────────────────────

void UHealthBarUI::TryBindToHealthComponent()
{
	if (BindToHealthComponent())
	{
		if (BindRetryTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		}

		UpdateHealthBar();
		UpdateHitCounter();
	}
	else
	{
		if (!BindRetryTimerHandle.IsValid())
		{
			UE_LOG(LogSideRunner, Warning,
				TEXT("HealthBarUI: Failed to bind — retrying every 0.1s"));

			GetWorld()->GetTimerManager().SetTimer(
				BindRetryTimerHandle,
				this,
				&UHealthBarUI::TryBindToHealthComponent,
				0.1f,
				true);
		}
	}
}

bool UHealthBarUI::BindToHealthComponent()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return false;

	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return false;

	OwningCharacter = Cast<ARunnerCharacter>(Pawn);
	if (!OwningCharacter) return false;

	UPlayerHealth* Health = OwningCharacter->HealthComponent;
	if (!Health) return false;

	ObservedHealth = MakeWeakObjectPtr(Health);

	Health->OnHealthChanged.AddDynamic(this, &UHealthBarUI::OnHealthChanged);
	Health->OnTakeDamage.AddDynamic(this, &UHealthBarUI::OnTakeDamage);
	Health->OnPlayerDeath.AddDynamic(this, &UHealthBarUI::OnPlayerDeath);

	CurrentHealth = Health->GetCurrentHealth();
	MaxHealth = Health->GetMaxHealthFloat();
	HitCount = Health->GetTotalHitsTaken();
	NormalizedHealth = Health->GetHealthPercent();

	return true;
}

void UHealthBarUI::UnbindFromHealthComponent()
{
	if (ObservedHealth.IsValid())
	{
		UPlayerHealth* Health = ObservedHealth.Get();
		Health->OnHealthChanged.RemoveDynamic(this, &UHealthBarUI::OnHealthChanged);
		Health->OnTakeDamage.RemoveDynamic(this, &UHealthBarUI::OnTakeDamage);
		Health->OnPlayerDeath.RemoveDynamic(this, &UHealthBarUI::OnPlayerDeath);
	}

	ObservedHealth.Reset();
	OwningCharacter = nullptr;

	if (BindRetryTimerHandle.IsValid())
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(BindRetryTimerHandle);
		}
		BindRetryTimerHandle.Invalidate();
	}
}

bool UHealthBarUI::ValidateWidgetBindings() const
{
	bool bValid = true;

	if (!HealthProgressBar)
	{
		UE_LOG(LogSideRunner, Error,
			TEXT("HealthBarUI: HealthProgressBar is not bound! "
				"Add a ProgressBar named 'HealthProgressBar' in your UMG Designer."));
		bValid = false;
	}

	if (!HitCounterText)
	{
		UE_LOG(LogSideRunner, Warning,
			TEXT("HealthBarUI: HitCounterText is not bound (optional). "
				"Add a TextBlock named 'HitCounterText' for hit counter."));
	}

	return bValid;
}
