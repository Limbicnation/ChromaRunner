#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "SideRunner.h" // Custom log categories
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "SideRunnerGameInstance.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Cache game instance
    CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

    // CRITICAL: Comprehensive widget binding validation with detailed diagnostic logging
    UE_LOG(LogSideRunner, Warning, TEXT("=== GameOverWidget Binding Validation ==="));

    bool bAllBindingsValid = true;

    // Validate text blocks
    if (!GameOverText)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ GameOverText is NULL - Add a TextBlock named 'GameOverText' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ GameOverText found"));
    }

    if (!ScoreText)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ ScoreText is NULL - Add a TextBlock named 'ScoreText' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ ScoreText found"));
    }

    if (!DistanceText)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ DistanceText is NULL - Add a TextBlock named 'DistanceText' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ DistanceText found"));
    }

    if (!HighScoreText)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ HighScoreText is NULL - Add a TextBlock named 'HighScoreText' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ HighScoreText found"));
    }

    if (!LivesText)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ LivesText is NULL - Add a TextBlock named 'LivesText' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ LivesText found"));
    }

    // Validate buttons
    if (!RestartButton)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ RestartButton is NULL - Add a Button named 'RestartButton' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        RestartButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ RestartButton found and bound"));
    }

    if (!QuitButton)
    {
        UE_LOG(LogSideRunner, Error, TEXT("  ❌ QuitButton is NULL - Add a Button named 'QuitButton' in WBP_GameOver"));
        bAllBindingsValid = false;
    }
    else
    {
        QuitButton->OnClicked.AddDynamic(this, &UGameOverWidget::OnQuitClicked);
        UE_LOG(LogSideRunner, Log, TEXT("  ✓ QuitButton found and bound"));
    }

    if (bAllBindingsValid)
    {
        UE_LOG(LogSideRunner, Log, TEXT("=== All widget bindings valid ✓ ==="));
    }
    else
    {
        UE_LOG(LogSideRunner, Error, TEXT("=== MISSING WIDGET ELEMENTS! See GAME_OVER_WIDGET_SETUP.md for setup guide ==="));
    }

    // Show mouse cursor for button interaction
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->bShowMouseCursor = true;
        PC->bEnableClickEvents = true;
        PC->bEnableMouseOverEvents = true;
    }
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

    // IMPROVED: Restore game input mode fully
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        FInputModeGameOnly InputMode;
        PC->SetInputMode(InputMode);
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
        UE_LOG(LogSideRunner, Error, TEXT("GameOverWidget: GameOverText not bound!"));
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

    UE_LOG(LogSideRunner, Log, TEXT("GameOverWidget display setup - Won: %s, Score: %d, Distance: %.1fm, Lives: %d"),
        bWon ? TEXT("Yes") : TEXT("No"), FinalScore, DistanceMeters, LivesUsed);
}

void UGameOverWidget::OnRestartClicked()
{
    UE_LOG(LogSideRunner, Log, TEXT("Restart button clicked"));

    // IMPROVED: Use IsValid() for UE5.5 best practices
    if (!IsValid(CachedGameInstance))
    {
        CachedGameInstance = Cast<USideRunnerGameInstance>(GetGameInstance());

        if (!IsValid(CachedGameInstance))
        {
            UE_LOG(LogSideRunner, Error, TEXT("GameOverWidget: Cannot restart - GameInstance is invalid!"));
            return;
        }
    }

    // CRITICAL FIX: Unpause the game before level reload
    UGameplayStatics::SetGamePaused(this, false);
    UE_LOG(LogSideRunner, Log, TEXT("GameOverWidget: Game unpaused for level restart"));

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
        UE_LOG(LogSideRunner, Error, TEXT("GameOverWidget: Cannot restart - World is null!"));
    }
}

void UGameOverWidget::OnQuitClicked()
{
    UE_LOG(LogSideRunner, Log, TEXT("Quit button clicked"));

    // CRITICAL FIX: Unpause the game before quitting
    UGameplayStatics::SetGamePaused(this, false);

    // Remove widget
    RemoveFromParent();

    // Quit game
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
    }
}
