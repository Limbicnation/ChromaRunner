#include "GameHUDWidget.h"
#include "Components/TextBlock.h"
#include "SideRunnerGameInstance.h"

void UGameHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache game instance
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    if (!CachedGameInstance)
    {
        UE_LOG(LogTemp, Error, TEXT("GameHUDWidget: Failed to get SideRunnerGameInstance!"));
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

    UE_LOG(LogTemp, Log, TEXT("GameHUDWidget constructed and delegates bound"));
}

void UGameHUDWidget::NativeDestruct()
{
    // Unbind delegates to prevent stale references
    if (CachedGameInstance && CachedGameInstance->IsValidLowLevel())
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
        if (CurrentLives <= 1)
        {
            LivesText->SetColorAndOpacity(FSlateColor(FLinearColor::Red)); // Critical
        }
        else if (CurrentLives <= 2)
        {
            LivesText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow)); // Warning
        }
        else
        {
            LivesText->SetColorAndOpacity(FSlateColor(FLinearColor::White)); // Normal
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameHUDWidget: LivesText not bound!"));
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
        UE_LOG(LogTemp, Error, TEXT("GameHUDWidget: ScoreText not bound!"));
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
        UE_LOG(LogTemp, Error, TEXT("GameHUDWidget: DistanceText not bound!"));
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
