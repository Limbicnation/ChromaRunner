# ChromaRunner Minimal Implementation Plan

## Executive Summary
This document outlines the minimal implementation for completing ChromaRunner with exactly three features:
1. **Enemy System** - Simplified enemy based on existing WallSpike
2. **Scoring System** - Distance-based scoring with coin bonuses
3. **End Condition** - Win at 5000m, lose on death

**Total Estimated Time: 4-6 hours** (experienced UE5 developer)

## Architecture Analysis

### Existing Systems We'll Reuse
- **Health System** (`PlayerHealthComponent`) - Already handles damage, invulnerability, death delegates
- **Damage Types** - `EDamageType::EnemyMelee` already exists but unused
- **Coin System** (`CoinCounter`) - Works perfectly for bonus scoring
- **Death/Restart** - Already implemented in `RunnerCharacter::HandlePlayerDeath()`
- **Level Generation** (`BaseLevel`, `SpawnLevel`) - 6 level variants working
- **UI Foundation** (`HealthBarWidget`) - Can be extended for score display

### What We DON'T Need (Avoiding Scope Creep)
- ❌ NO new damage types or complex AI
- ❌ NO save/load system
- ❌ NO main menu or pause menu
- ❌ NO power-ups or upgrades
- ❌ NO leaderboards or achievements
- ❌ NO complex enemy behaviors

## Feature 1: Minimal Enemy System

### Decision: Transform WallSpike into True Enemy
**Rationale**: WallSpike already has:
- Chase behavior (follows player)
- Collision detection
- Damage dealing (instant death)
- Self-cleanup when behind player
- Audio support

### Implementation: ASimpleEnemy Class
```cpp
// Source/SideRunner/SimpleEnemy.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleEnemy.generated.h"

UCLASS()
class SIDERUNNER_API ASimpleEnemy : public AActor
{
    GENERATED_BODY()

public:
    ASimpleEnemy();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UBoxComponent* CollisionBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* EnemyMesh;

    // Enemy Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "100.0", ClampMax = "800.0"))
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "10.0", ClampMax = "100.0"))
    int32 ContactDamage = 25; // 4 hits to kill player

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    bool bPatrolMode = true; // Simple back-and-forth movement

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float PatrolDistance = 400.0f;

private:
    class ARunnerCharacter* PlayerRef;
    FVector StartLocation;
    int32 PatrolDirection = 1;
    float PatrolTimer = 0.0f;
    bool bHasDealtDamage = false; // Prevent multi-hit

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    void SimplePatrolMovement(float DeltaTime);
    void CleanupIfBehindPlayer();
};
```

### Key Methods Implementation
```cpp
void ASimpleEnemy::SimplePatrolMovement(float DeltaTime)
{
    // Ultra-simple patrol: move back and forth
    FVector CurrentPos = GetActorLocation();
    float DistanceFromStart = FVector::Dist2D(CurrentPos, StartLocation);

    if (DistanceFromStart >= PatrolDistance)
    {
        PatrolDirection *= -1; // Reverse direction
    }

    // Move along Y-axis (side-scrolling direction)
    FVector Movement = FVector(0, PatrolDirection * MoveSpeed * DeltaTime, 0);
    SetActorLocation(CurrentPos + Movement);
}

void ASimpleEnemy::OnOverlapBegin(...)
{
    ARunnerCharacter* Player = Cast<ARunnerCharacter>(OtherActor);
    if (Player && !bHasDealtDamage)
    {
        if (UPlayerHealthComponent* HealthComp = Player->HealthComponent)
        {
            HealthComp->TakeDamage(ContactDamage, EDamageType::EnemyMelee);
            bHasDealtDamage = true; // Prevent multi-hit

            // Reset after invulnerability
            FTimerHandle ResetTimer;
            GetWorldTimerManager().SetTimer(ResetTimer, [this]() {
                bHasDealtDamage = false;
            }, 1.5f, false);
        }
    }
}
```

### Blueprint Integration
- Create `BP_SimpleEnemy` extending `ASimpleEnemy`
- Set mesh to existing spike or cube mesh
- Place 2-3 enemies per level segment
- Configure patrol distance based on platform width

**Time Estimate: 1.5 hours**

## Feature 2: Minimal Scoring System

### Decision: Create USideRunnerGameInstance
**Rationale**: GameInstance persists across level reloads, perfect for score tracking

### Implementation: USideRunnerGameInstance Class
```cpp
// Source/SideRunner/SideRunnerGameInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SideRunnerGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScoreUpdated, int32, NewScore);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDistanceUpdated, float, NewDistance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameWon);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameLost);

UCLASS()
class SIDERUNNER_API USideRunnerGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    virtual void Init() override;

    // Score Management
    UFUNCTION(BlueprintCallable, Category = "Score")
    void UpdateDistanceScore(float PlayerXPosition);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddCoinBonus(int32 CoinValue = 10);

    UFUNCTION(BlueprintCallable, Category = "Score")
    void AddEnemyKillBonus(int32 BonusValue = 50);

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetCurrentScore() const { return CurrentScore; }

    UFUNCTION(BlueprintPure, Category = "Score")
    float GetDistanceTraveled() const { return DistanceTraveled; }

    UFUNCTION(BlueprintPure, Category = "Score")
    int32 GetHighScore() const { return HighScore; }

    // Game State
    UFUNCTION(BlueprintCallable, Category = "Game")
    void CheckWinCondition();

    UFUNCTION(BlueprintCallable, Category = "Game")
    void TriggerGameOver(bool bWon);

    UFUNCTION(BlueprintCallable, Category = "Game")
    void ResetGameSession();

    // Events
    UPROPERTY(BlueprintAssignable)
    FOnScoreUpdated OnScoreUpdated;

    UPROPERTY(BlueprintAssignable)
    FOnDistanceUpdated OnDistanceUpdated;

    UPROPERTY(BlueprintAssignable)
    FOnGameWon OnGameWon;

    UPROPERTY(BlueprintAssignable)
    FOnGameLost OnGameLost;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 CurrentScore = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Score")
    float DistanceTraveled = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Score")
    int32 HighScore = 0;

    UPROPERTY(EditDefaultsOnly, Category = "Game", meta = (ClampMin = "1000.0", ClampMax = "10000.0"))
    float WinDistance = 5000.0f; // 5000 meters to win

private:
    float LastRecordedX = 0.0f;
    bool bGameEnded = false;
};
```

### Key Methods Implementation
```cpp
void USideRunnerGameInstance::UpdateDistanceScore(float PlayerXPosition)
{
    if (bGameEnded) return;

    // Only count forward progress
    if (PlayerXPosition > LastRecordedX)
    {
        float DeltaDistance = PlayerXPosition - LastRecordedX;
        DistanceTraveled += DeltaDistance;

        // 1 point per meter
        int32 DistancePoints = FMath::FloorToInt(DeltaDistance / 100.0f); // Unreal units to meters
        if (DistancePoints > 0)
        {
            CurrentScore += DistancePoints;
            OnScoreUpdated.Broadcast(CurrentScore);
        }

        LastRecordedX = PlayerXPosition;
        OnDistanceUpdated.Broadcast(DistanceTraveled / 100.0f); // Convert to meters for display

        CheckWinCondition();
    }
}

void USideRunnerGameInstance::CheckWinCondition()
{
    if (DistanceTraveled >= WinDistance * 100.0f && !bGameEnded) // Convert meters to Unreal units
    {
        TriggerGameOver(true);
    }
}

void USideRunnerGameInstance::TriggerGameOver(bool bWon)
{
    if (bGameEnded) return;

    bGameEnded = true;

    if (CurrentScore > HighScore)
    {
        HighScore = CurrentScore;
    }

    if (bWon)
    {
        OnGameWon.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("GAME WON! Distance: %.1fm, Score: %d"),
            DistanceTraveled / 100.0f, CurrentScore);
    }
    else
    {
        OnGameLost.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("GAME LOST! Distance: %.1fm, Score: %d"),
            DistanceTraveled / 100.0f, CurrentScore);
    }
}
```

### Integration with RunnerCharacter
```cpp
// In RunnerCharacter::Tick()
if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
{
    GameInstance->UpdateDistanceScore(GetActorLocation().X);
}

// In RunnerCharacter::HandlePlayerDeath()
if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
{
    GameInstance->TriggerGameOver(false);
}
```

**Time Estimate: 1.5 hours**

## Feature 3: End Condition Implementation

### Win Condition Display
```cpp
// Source/SideRunner/GameOverWidget.h
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

UCLASS()
class SIDERUNNER_API UGameOverWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetupGameOverDisplay(bool bWon, int32 Score, float Distance);

protected:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* GameOverText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* ScoreText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DistanceText;

    UPROPERTY(meta = (BindWidget))
    class UButton* RestartButton;

    UPROPERTY(meta = (BindWidget))
    class UButton* QuitButton;

    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnRestartClicked();

    UFUNCTION()
    void OnQuitClicked();
};
```

### Implementation
```cpp
void UGameOverWidget::SetupGameOverDisplay(bool bWon, int32 Score, float Distance)
{
    if (GameOverText)
    {
        FString Message = bWon ? TEXT("YOU WIN!") : TEXT("GAME OVER");
        GameOverText->SetText(FText::FromString(Message));

        // Set color based on win/lose
        FSlateColor Color = bWon ? FSlateColor(FLinearColor::Green) : FSlateColor(FLinearColor::Red);
        GameOverText->SetColorAndOpacity(Color);
    }

    if (ScoreText)
    {
        FString ScoreString = FString::Printf(TEXT("Score: %d"), Score);
        ScoreText->SetText(FText::FromString(ScoreString));
    }

    if (DistanceText)
    {
        FString DistString = FString::Printf(TEXT("Distance: %.1f m"), Distance);
        DistanceText->SetText(FText::FromString(DistString));
    }
}

void UGameOverWidget::OnRestartClicked()
{
    // Reset game instance
    if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
    {
        GameInstance->ResetGameSession();
    }

    // Reload level
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}
```

**Time Estimate: 1 hour**

## Implementation Order & Dependencies

### Phase 1: Core Systems (1.5 hours)
1. Create `USideRunnerGameInstance` class
2. Update project settings to use custom GameInstance
3. Integrate distance tracking in `RunnerCharacter::Tick()`
4. Test basic scoring

### Phase 2: Enemy System (1.5 hours)
1. Create `ASimpleEnemy` class
2. Implement patrol movement and collision
3. Create `BP_SimpleEnemy` blueprint
4. Place enemies in level segments
5. Test damage and invulnerability

### Phase 3: UI & End Conditions (1 hour)
1. Create `UGameOverWidget` class
2. Create `WBP_GameOver` widget blueprint
3. Hook up win/lose events
4. Create simple HUD for score/distance display
5. Test full game loop

### Phase 4: Polish & Testing (1 hour)
1. Balance enemy damage (25 HP = 4 hits)
2. Adjust win distance if needed
3. Ensure score persistence works
4. Fix any edge cases

## File Structure
```
Source/SideRunner/
├── SimpleEnemy.h/cpp           [NEW]
├── SideRunnerGameInstance.h/cpp [NEW]
├── GameOverWidget.h/cpp        [NEW]
├── RunnerCharacter.cpp         [MODIFY: Add score tracking]
├── SideRunner.Build.cs         [No changes needed]

Content/Blueprints/
├── BP_SimpleEnemy.uasset       [NEW]
├── BP_GameInstance.uasset     [NEW]
├── WBP_GameOver.uasset        [NEW]
├── WBP_ScoreHUD.uasset        [NEW]
```

## Critical Success Factors

### Performance Considerations
- Enemies use simple patrol, no pathfinding (mobile-friendly)
- Score updates throttled to every 100 units
- Reuse existing collision channels
- Object pooling not needed (max 10 enemies on screen)

### Network Replication Readiness
- GameInstance naturally supports multiplayer migration
- Score/distance can easily add `UPROPERTY(Replicated)`
- Enemy state machine simple enough for prediction

### Testing Checklist
- [ ] Player can take 4 hits before death
- [ ] Score increases with distance
- [ ] Coins add 10 points each
- [ ] Game wins at 5000m
- [ ] Game over screen shows correct info
- [ ] Restart button works
- [ ] Score persists between deaths
- [ ] Enemies patrol correctly
- [ ] Invulnerability frames prevent multi-hit

## Alternative Approaches Considered

### Why NOT Extend WallSpike?
- WallSpike has complex chase AI we don't need
- Instant death too punishing for basic enemy
- Audio system overhead unnecessary
- Cleaner to have dedicated simple enemy class

### Why NOT Use GameMode?
- GameMode resets on level reload
- GameInstance persists naturally
- Better for score tracking across deaths
- Standard UE5 pattern for persistent data

### Why Distance Not Time?
- Distance rewards skillful play
- Time would encourage camping
- Distance naturally increases difficulty
- More intuitive for players

## Code Snippets for Critical Integration Points

### RunnerCharacter.cpp Modification
```cpp
// Add to Tick() function, after camera update
// Around line 132
if (!IsDead())
{
    if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
    {
        GameInstance->UpdateDistanceScore(GetActorLocation().X);
    }
}

// Modify HandlePlayerDeath()
// Around line 435, before DeathOfPlayer()
if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
{
    GameInstance->TriggerGameOver(false);
}
```

### CoinPickup Integration
```cpp
// In CoinPickup::NotifyActorBeginOverlap() or wherever coins are collected
if (USideRunnerGameInstance* GameInstance = Cast<USideRunnerGameInstance>(GetGameInstance()))
{
    GameInstance->AddCoinBonus(10); // 10 points per coin
}
```

## Final Notes

This plan delivers EXACTLY what was requested:
1. **Enemies** - Simple patrol enemies that deal damage
2. **Scoring** - Distance + coin bonuses
3. **End Condition** - Win at 5000m, lose on death

No scope creep, no extra features, just the minimal viable game completion.

**Total Implementation Time: 4-6 hours** for an experienced UE5 developer
**Testing & Polish: 1-2 hours** additional

The architecture is clean, performant, and ready for future expansion if needed, but complete as-is for the minimal requirements.