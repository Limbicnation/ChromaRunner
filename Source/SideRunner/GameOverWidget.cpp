#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SideRunnerGameInstance.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache game instance
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    // Bind button click events
    if (RestartButton)
    {
        RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameOverWidget: RestartButton not found! Check UMG widget binding."));
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameOverWidget: QuitButton not found! Check UMG widget binding."));
    }

    // Show mouse cursor for button interaction
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }

    UE_LOG(LogTemp, Log, TEXT("GameOverWidget constructed and buttons bound"));
}

void UGameOverWidget::NativeDestruct()
{
    // Clean up button bindings
    if (RestartButton)
    {
        RestartButton->OnClicked.RemoveAll(this);
    }

    if (QuitButton)
    {
        QuitButton->OnClicked.RemoveAll(this);
    }

    // Hide mouse cursor
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->bShowMouseCursor = false;
        PC->bEnableClickEvents = false;
        PC->bEnableMouseOverEvents = false;
    }

    Super::NativeDestruct();
}

void UGameOverWidget::SetupGameOverDisplay(bool bWon, int32 FinalScore, float DistanceMeters, int32 HighScore, int32 LivesUsed)
{
    // Set main game over text
    if (GameOverText)
    {
        const FString Message = bWon ? TEXT("YOU WIN!") : TEXT("GAME OVER");
        GameOverText->SetText(FText::FromString(Message));

        // Set color based on win/lose
        const FSlateColor Color = bWon ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red);
        GameOverText->SetColorAndOpacity(Color);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameOverWidget: GameOverText not bound!"));
    }

    // Set score text
    if (ScoreText)
    {
        const FString ScoreString = FString::Printf(TEXT("Final Score: %d"), FinalScore);
        ScoreText->SetText(FText::FromString(ScoreString));
    }

    // Set distance text
    if (DistanceText)
    {
        const FString DistString = FString::Printf(TEXT("Distance: %.1f m"), DistanceMeters);
        DistanceText->SetText(FText::FromString(DistString));
    }

    // Set high score text
    if (HighScoreText)
    {
        const FString HighScoreString = FString::Printf(TEXT("High Score: %d"), HighScore);
        HighScoreText->SetText(FText::FromString(HighScoreString));

        // Highlight if new high score
        if (FinalScore >= HighScore && FinalScore > 0)
        {
            HighScoreText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.84f, 0.0f))); // Gold color
        }
    }

    // Set lives used text
    if (LivesText)
    {
        const FString LivesString = FString::Printf(TEXT("Lives Used: %d"), LivesUsed);
        LivesText->SetText(FText::FromString(LivesString));
    }

    UE_LOG(LogTemp, Log, TEXT("GameOverWidget display setup - Won: %s, Score: %d, Distance: %.1fm, Lives: %d"),
        bWon ? TEXT("Yes") : TEXT("No"), FinalScore, DistanceMeters, LivesUsed);
}

void UGameOverWidget::OnRestartClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Restart button clicked"));

    // Validate game instance
    if (!CachedGameInstance || !CachedGameInstance->IsValidLowLevel())
    {
        CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

        if (!CachedGameInstance)
        {
            UE_LOG(LogTemp, Error, TEXT("GameOverWidget: Cannot restart - GameInstance is invalid!"));
            return;
        }
    }

    // Reset game session (clears score, distance, resets lives to 3)
    CachedGameInstance->ResetGameSession();

    // Remove this widget from viewport
    RemoveFromParent();

    // Reload current level
    const UWorld* World = GetWorld();
    if (World)
    {
        const FString CurrentLevelName = World->GetName();
        UGameplayStatics::OpenLevel(this, FName(*CurrentLevelName));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GameOverWidget: Cannot restart - World is null!"));
    }
}

void UGameOverWidget::OnQuitClicked()
{
    UE_LOG(LogTemp, Log, TEXT("Quit button clicked"));

    // Remove widget
    RemoveFromParent();

    // Quit game
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}
