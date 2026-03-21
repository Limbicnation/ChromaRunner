#include "GameOverWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void UGameOverWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Bind button clicks
    if (btn_Restart)
    {
        btn_Restart->OnClicked.AddDynamic(this, &UGameOverWidget::OnRestartClicked);
    }

    if (btn_MainMenu)
    {
        btn_MainMenu->OnClicked.AddDynamic(this, &UGameOverWidget::OnMainMenuClicked);
    }
}

void UGameOverWidget::SetFinalScore(int32 Score)
{
    FinalScore = Score;

    if (txt_FinalScore)
    {
        txt_FinalScore->SetText(FText::FormatOrdered(
            FText::FromString(TEXT("SCORE: {0}")),
            FText::AsNumber(Score)
        ));
    }
}

void UGameOverWidget::OnRestartClicked()
{
    // Unpause the game before reloading
    UGameplayStatics::SetGamePaused(this, false);

    // Remove this widget
    RemoveFromParent();

    // Reload the current level
    const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevel), false);
}

void UGameOverWidget::OnMainMenuClicked()
{
    // Unpause the game before navigating
    UGameplayStatics::SetGamePaused(this, false);

    // Close this widget
    RemoveFromParent();

    // TODO: Navigate to main menu map instead of reloading current level
    const FString CurrentLevel = UGameplayStatics::GetCurrentLevelName(this, true);
    UGameplayStatics::OpenLevel(this, FName(*CurrentLevel), false);
}
