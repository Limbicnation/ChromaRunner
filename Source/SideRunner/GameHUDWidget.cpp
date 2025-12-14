#include "GameHUDWidget.h"
#include "Components/TextBlock.h"
#include "SideRunner.h" // Custom log categories
#include "SideRunnerGameInstance.h"

// UI Constants for Lives Display
namespace UIConstants
{
    constexpr int32 LIVES_CRITICAL_THRESHOLD = 1;
    constexpr int32 LIVES_WARNING_THRESHOLD = 2;

    const FLinearColor COLOR_CRITICAL = FLinearColor::Red;
    const FLinearColor COLOR_WARNING = FLinearColor::Yellow;
    const FLinearColor COLOR_NORMAL = FLinearColor::White;
}

void UGameHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache game instance
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    if (!CachedGameInstance)
    {
        UE_LOG(LogSideRunner, Error, TEXT("GameHUDWidget: Failed to get SideRunnerGameInstance!"));
        return;
    }

    // Bind to game instance delegates for automatic updates
    CachedGameInstance->OnLivesUpdated.AddDynamic(this, &UGameHUDWidget::OnLivesUpdatedHandler);
    CachedGameInstance->OnScoreUpdated.AddDynamic(this, &UGameHUDWidget::OnScoreUpdatedHandler);
    CachedGameInstance->OnDistanceUpdated.AddDynamic(this, &UGameHUDWidget::OnDistanceUpdatedHandler);

    // Initialize display with current values
    UpdateLivesDisplay(CachedGameInstance->GetCurrentLives(), CachedGameInstance->GetMaxLives());
    UpdateScoreDisplay(CachedGameInstance->GetCurrentScore());
    UpdateDistanceDisplay(CachedGameInstance->GetDistanceTraveled());

    UE_LOG(LogSideRunner, Log, TEXT("GameHUDWidget constructed and delegates bound"));
}

void UGameHUDWidget::NativeDestruct()
{
    // Unbind delegates to prevent stale references
    if (IsValid(CachedGameInstance))
    {
        CachedGameInstance->OnLivesUpdated.RemoveAll(this);
        CachedGameInstance->OnScoreUpdated.RemoveAll(this);
        CachedGameInstance->OnDistanceUpdated.RemoveAll(this);
    }

    Super::NativeDestruct();
}

void UGameHUDWidget::UpdateLivesDisplay(int32 CurrentLives, int32 MaxLives)
{
    if (LivesText)
    {
        const FString LivesString = FString::Printf(TEXT("Lives: %d/%d"), CurrentLives, MaxLives);
        LivesText->SetText(FText::FromString(LivesString));

        // Change color based on lives remaining
        if (CurrentLives <= UIConstants::LIVES_CRITICAL_THRESHOLD)
        {
            LivesText->SetColorAndOpacity(FSlateColor(UIConstants::COLOR_CRITICAL));
        }
        else if (CurrentLives <= UIConstants::LIVES_WARNING_THRESHOLD)
        {
            LivesText->SetColorAndOpacity(FSlateColor(UIConstants::COLOR_WARNING));
        }
        else
        {
            LivesText->SetColorAndOpacity(FSlateColor(UIConstants::COLOR_NORMAL));
        }
    }
    else
    {
        UE_LOG(LogSideRunner, Error, TEXT("GameHUDWidget: LivesText not bound!"));
    }
}

void UGameHUDWidget::UpdateScoreDisplay(int32 CurrentScore)
{
    if (ScoreText)
    {
        const FString ScoreString = FString::Printf(TEXT("Score: %d"), CurrentScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }
    else
    {
        UE_LOG(LogSideRunner, Error, TEXT("GameHUDWidget: ScoreText not bound!"));
    }
}

void UGameHUDWidget::UpdateDistanceDisplay(float DistanceMeters)
{
    if (DistanceText)
    {
        const FString DistString = FString::Printf(TEXT("Distance: %.0f m"), DistanceMeters);
        DistanceText->SetText(FText::FromString(DistString));
    }
    else
    {
        UE_LOG(LogSideRunner, Error, TEXT("GameHUDWidget: DistanceText not bound!"));
    }
}

void UGameHUDWidget::OnLivesUpdatedHandler(int32 CurrentLives, int32 MaxLives)
{
    UpdateLivesDisplay(CurrentLives, MaxLives);
}

void UGameHUDWidget::OnScoreUpdatedHandler(int32 NewScore)
{
    UpdateScoreDisplay(NewScore);
}

void UGameHUDWidget::OnDistanceUpdatedHandler(float NewDistance)
{
    UpdateDistanceDisplay(NewDistance);
}
