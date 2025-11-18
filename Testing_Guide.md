# ChromaRunner - Game Over UMG System Testing Guide

This guide provides comprehensive testing procedures for validating the Phase 4 Game Over UMG system implementation.

## Prerequisites

✅ **C++ Code Compiled**: All Phase 4 C++ classes built successfully in Visual Studio
✅ **Blueprint Widgets Created**: WBP_GameHUD and WBP_GameOver created following `Blueprint_UMG_Setup_Guide.md`
✅ **GameMode Configured**: BP_SideRunnerGameMode set as default with widget classes assigned

---

## Quick Start Testing

### Test 1: Win Condition (5000m Distance)

**Objective**: Verify the game over screen appears when player reaches 5000 meters

**Steps**:
1. Launch game in PIE (Play In Editor)
2. Wait for game to start and HUD to appear
3. Press **`** (tilde key) to open console
4. Type: `TeleportToDistance 5000`
5. Press Enter

**Expected Results**:
- ✅ Game Over screen appears within 1 second
- ✅ Header text displays "YOU WIN!" in green color
- ✅ Final score shows correctly
- ✅ Distance displays "5000.0 m"
- ✅ High score highlighted in gold if new record
- ✅ RESTART and QUIT buttons visible and clickable
- ✅ Mouse cursor appears automatically

**Pass Criteria**: All expected results met

---

### Test 2: Loss Condition (Lives Exhausted)

**Objective**: Verify game over screen appears when lives reach 0

**Steps**:
1. Launch game in PIE
2. Wait for game start
3. Open console (` key)
4. Type: `KillPlayer` and press Enter
5. Repeat `KillPlayer` command 2 more times (total 3 deaths)

**Expected Results**:
- ✅ After 1st death: Player respawns, lives show 2/3
- ✅ After 2nd death: Player respawns, lives show 1/3
- ✅ After 3rd death: Game Over screen appears
- ✅ Header text displays "GAME OVER" in red color
- ✅ Final stats displayed (score, distance, high score, lives used: 3)
- ✅ Motivational message appears (e.g., "You can do this! One more try?")
- ✅ RESTART and QUIT buttons functional

**Pass Criteria**: All expected results met

---

### Test 3: Restart Functionality

**Objective**: Verify restart button reloads level with reset state

**Steps**:
1. Trigger game over (either win or loss)
2. Click **RESTART** button on Game Over screen

**Expected Results**:
- ✅ Level reloads within 2-3 seconds
- ✅ Lives reset to 3/3
- ✅ Score reset to 0
- ✅ Distance reset to 0 m
- ✅ Player spawns at starting position
- ✅ HUD displays correctly with reset values
- ✅ Input mode switches to game (mouse cursor hidden)

**Pass Criteria**: All expected results met, no errors in Output Log

---

## Comprehensive Test Scenarios

### HUD Testing

#### Test 4: Lives Display Updates

**Objective**: Verify lives counter updates correctly on damage/death

**Steps**:
1. Start game
2. Note initial lives display (should show 3/3)
3. Use `KillPlayer` console command
4. Observe lives display

**Expected Results**:
- ✅ Lives display updates immediately after death
- ✅ Display shows correct format: "X/3" where X is remaining lives
- ✅ Color changes:
  - White when 3 lives remain
  - Yellow when 2 lives remain
  - Red when 1 life remains

**Validation**:
```
3/3 → KillPlayer → 2/3 (yellow) → KillPlayer → 1/3 (red) → KillPlayer → Game Over
```

---

#### Test 5: Score Counter Updates

**Objective**: Verify score updates in real-time during gameplay

**Steps**:
1. Start game
2. Move forward (distance scoring)
3. Collect coins (if available in level)
4. Monitor score display

**Expected Results**:
- ✅ Score increases as player moves forward (1 point per meter)
- ✅ Score updates smoothly without flickering
- ✅ Coin collection adds +10 points
- ✅ Score display format: "SCORE: X,XXX" with comma separators (for scores > 999)

**Console Validation**:
- Check Output Log for: `"Distance score updated"` messages
- Verify score calculation: `Score = (Distance in meters) + (Coins × 10)`

---

#### Test 6: Distance Counter Updates

**Objective**: Verify distance counter increments as player progresses

**Steps**:
1. Start game
2. Let character run forward for 10 seconds
3. Observe distance counter

**Expected Results**:
- ✅ Distance counter updates smoothly every frame
- ✅ Display format: "Distance: X m" or "Distance: X.X m"
- ✅ Counter increments continuously during forward movement
- ✅ Counter does not decrement when moving backward

**Validation**:
- Use `TeleportToDistance 100` → Distance should show ~100m
- Use `TeleportToDistance 2500` → Distance should show ~2500m

---

### Game Over Screen Testing

#### Test 7: Win State Visual Design

**Objective**: Verify win state displays correct colors and celebration elements

**Steps**:
1. Use `TeleportToDistance 5000` to trigger win
2. Examine Game Over screen appearance

**Expected Visual Elements**:
- ✅ "YOU WIN!" text in **green** color (#4CAF50 or similar)
- ✅ Final score displayed prominently
- ✅ Distance shows "5000.0 m" or higher
- ✅ High score comparison (if new record, highlighted in gold)
- ✅ Lives Used display (e.g., "Lives Used: 1")
- ✅ Background semi-transparent overlay
- ✅ RESTART button in green (#4CAF50)
- ✅ QUIT button in gray (#757575)

**Optional Polish** (if implemented):
- Particle effects (confetti/sparkles)
- Victory sound/music
- Animated stat reveal

---

#### Test 8: Loss State Visual Design

**Objective**: Verify loss state displays correct colors and motivational messaging

**Steps**:
1. Use `KillPlayer` ×3 to trigger loss
2. Examine Game Over screen appearance

**Expected Visual Elements**:
- ✅ "GAME OVER" text in **red** color (#E53935 or similar)
- ✅ Final score displayed
- ✅ Distance traveled shown (likely < 5000m)
- ✅ High score comparison
- ✅ Lives Used: 3/3
- ✅ **Motivational Message** appears:
  - Examples: "You can do this! One more try?", "Great progress!", "Almost there!"
  - Color: Yellow (#FFEB3B) or cyan
- ✅ RESTART button in blue (#2196F3)
- ✅ QUIT button in gray

---

#### Test 9: High Score Detection

**Objective**: Verify high score tracking and display across sessions

**Steps**:
1. Start game and achieve a score (e.g., 500 points)
2. Trigger game over
3. Note high score on Game Over screen
4. Restart game
5. Achieve higher score (e.g., 1000 points)
6. Trigger game over again

**Expected Results**:
- ✅ First game over: High score shows 500
- ✅ Second game over: High score shows 1000 (updated)
- ✅ High score text highlighted in **gold** color if new record
- ✅ Previous high score persists across game restarts (saved in GameInstance)

**Validation**:
- Check Output Log for: `"UpdateHighScore: New high score: X"`
- Verify high score increases but never decreases

---

### Input Mode Testing

#### Test 10: Input Mode Switching (Game ↔ UI)

**Objective**: Verify input modes switch correctly between gameplay and UI interaction

**Steps**:
1. Start game (input mode: Game)
2. Trigger game over (input mode: should switch to UI)
3. Click RESTART (input mode: should switch back to Game)

**Expected Results**:

**During Gameplay**:
- ✅ Mouse cursor hidden
- ✅ Keyboard controls player movement
- ✅ No click events on screen

**During Game Over**:
- ✅ Mouse cursor visible and movable
- ✅ Buttons respond to hover (color change/scale)
- ✅ Buttons respond to click
- ✅ Player movement controls disabled

**After Restart**:
- ✅ Mouse cursor hidden again
- ✅ Keyboard controls re-enabled
- ✅ No lingering UI click events

**Console Validation**:
- Check Output Log for:
  - `"SideRunnerGameMode: Input mode set to UI"`
  - `"SideRunnerGameMode: Input mode set to Game"`

---

### Widget Lifecycle Testing

#### Test 11: Widget Creation and Destruction

**Objective**: Verify widgets are created/destroyed properly without memory leaks

**Steps**:
1. Start game
2. Trigger game over
3. Restart game
4. Repeat 5 times (5 full cycles)

**Expected Results**:
- ✅ Game HUD appears every time on game start
- ✅ Game Over screen appears every time on game over
- ✅ No duplicate widgets (only one HUD, one Game Over at a time)
- ✅ Previous Game Over widget destroyed before new HUD created
- ✅ No errors in Output Log about widget creation failures

**Console Validation**:
Check Output Log for expected patterns (×5):
```
"SideRunnerGameMode: GameHUD created and added to viewport"
→ [gameplay]
→ "SideRunnerGameMode: Game Over screen displayed"
→ [restart]
→ "SideRunnerGameMode: GameHUD hidden"
→ [repeat]
```

**Memory Leak Check**:
- Monitor memory usage in Unreal Editor (Stat Memory)
- After 5 cycles, memory should not increase significantly (< 5 MB growth acceptable)

---

#### Test 12: Widget Binding Verification

**Objective**: Ensure all UMG widget bindings from Blueprint to C++ are correct

**Prerequisites**: WBP_GameHUD and WBP_GameOver created with proper bindings

**Steps**:
1. Open Output Log
2. Clear log (right-click → Clear)
3. Start game

**Expected Log Messages**:

**For WBP_GameHUD**:
```
GameHUDWidget: Successfully bound to GameInstance delegates
GameHUDWidget: Initial values set - Lives: 3/3, Score: 0, Distance: 0m
```

**For WBP_GameOver** (after triggering game over):
```
GameOverWidget constructed and buttons bound
GameOverWidget display setup - Won: [Yes/No], Score: X, Distance: Xm, Lives: X
```

**Failure Indicators** (should NOT appear):
```
Error: GameOverWidget: RestartButton not found! Check UMG widget binding.
Error: GameOverWidget: QuitButton not found! Check UMG widget binding.
Error: GameHUDWidget: LivesText not bound correctly!
```

---

### Edge Case Testing

#### Test 13: Exact Win Distance (5000m Boundary)

**Objective**: Verify win triggers at exactly 5000m (not 4999m or 5001m)

**Steps**:
1. Use console: `TeleportToDistance 4999`
2. Observe: Game should continue (no win screen)
3. Use console: `TeleportToDistance 5000`
4. Observe: Game Over (win) screen should appear

**Expected Results**:
- ✅ 4999m does NOT trigger win
- ✅ 5000m DOES trigger win
- ✅ Win condition uses `>=` comparison (distance ≥ 5000m)

**Code Validation**:
Refer to `SideRunnerGameInstance.cpp:126`:
```cpp
if (DistanceTraveled >= WinDistanceUnrealUnits)
{
    TriggerGameOver(true);
}
```

---

#### Test 14: Multiple Game Over Calls (Duplicate Prevention)

**Objective**: Verify duplicate game over screens don't appear

**Steps**:
1. Use console: `TeleportToDistance 5000` (triggers win)
2. Immediately spam console: `TeleportToDistance 6000` (should be ignored)

**Expected Results**:
- ✅ Only ONE Game Over screen appears
- ✅ Second win trigger is ignored (game already ended)
- ✅ Output Log shows:
  ```
  SideRunnerGameMode: Game over already active, ignoring duplicate call
  ```

**Code Validation**:
Refer to `SideRunnerGameMode.cpp:97`:
```cpp
if (bGameOverActive)
{
    UE_LOG(LogTemp, Warning, TEXT("...ignoring duplicate call"));
    return;
}
```

---

#### Test 15: Restart from Win vs Loss

**Objective**: Verify restart works identically from both win and loss states

**Steps**:
1. **Win Path**:
   - `TeleportToDistance 5000` → Win screen → Click RESTART
   - Verify game restarts correctly

2. **Loss Path**:
   - `KillPlayer` ×3 → Loss screen → Click RESTART
   - Verify game restarts correctly

**Expected Results** (both paths):
- ✅ Level reloads to starting state
- ✅ Lives = 3/3
- ✅ Score = 0
- ✅ Distance = 0m
- ✅ High score preserved (not reset)
- ✅ No errors in Output Log

**Consistency Check**:
Both paths should produce identical restart behavior - if one works and the other doesn't, there's a bug.

---

## Performance Testing

### Test 16: HUD Update Performance

**Objective**: Verify HUD updates don't cause frame rate drops

**Steps**:
1. Enable FPS counter: Console → `stat fps`
2. Play game normally for 30 seconds
3. Monitor frame rate

**Expected Results**:
- ✅ Frame rate remains stable (60 FPS target on decent hardware)
- ✅ No stuttering when score/distance updates
- ✅ No frame drops when Game Over screen appears

**Performance Targets**:
- HUD update time: < 0.1ms per frame
- Game Over widget creation: < 50ms
- Input mode switching: < 10ms

**Console Validation**:
```
stat game  → Check Game thread time (should be < 16.6ms for 60 FPS)
stat unit  → Check overall frame time
```

---

### Test 17: Memory Usage (Leak Detection)

**Objective**: Verify no memory leaks from repeated widget creation/destruction

**Steps**:
1. Note baseline memory: Console → `stat memory`
2. Complete 10 full game cycles (start → game over → restart ×10)
3. Check memory again: Console → `stat memory`

**Expected Results**:
- ✅ Memory increase < 10 MB after 10 cycles
- ✅ No continuous memory growth with each cycle
- ✅ Garbage collector reclaims widget memory

**Validation**:
If memory grows > 5 MB per cycle, there's likely a memory leak (check for:
- Widget not removed from viewport
- Delegates not unbound
- Strong references preventing GC)

---

## Integration Testing

### Test 18: Lives System Integration

**Objective**: Verify lives system integrates correctly with game over flow

**Steps**:
1. Start game
2. Take damage until 1st death (lives: 3 → 2)
3. Take damage until 2nd death (lives: 2 → 1)
4. Take damage until 3rd death (lives: 1 → 0 → GAME OVER)

**Expected Results**:
- ✅ Each death decrements lives by 1
- ✅ Player respawns after 1st and 2nd death
- ✅ Game over triggers only on 3rd death (lives = 0)
- ✅ Game Over screen shows "Lives Used: 3"

**Console Validation**:
```
Lives decremented - Remaining: 2/3
Lives decremented - Remaining: 1/3
Lives decremented - Remaining: 0/3
=== GAME OVER ===
```

---

### Test 19: Score System Integration

**Objective**: Verify all scoring components contribute correctly to final score

**Setup**: Ensure level has coins to collect

**Steps**:
1. Start game
2. Move forward to accumulate distance score
3. Collect 5 coins (+50 points)
4. Trigger game over

**Expected Final Score**:
```
Score = (Distance in meters × 1) + (Coins × 10) + (Enemies × 50)
```

**Example Calculation**:
- Distance: 1000m = 1000 points
- Coins: 5 = 50 points
- Enemies: 2 = 100 points (if implemented)
- **Total: 1150 points**

**Validation**:
Game Over screen should display correct breakdown (if implemented):
```
Distance:  1000m  ×  1pt  = 1000
Coins:        5   × 10pt  =   50
Enemies:      2   × 50pt  =  100
──────────────────────────────
Total Score:             = 1150
```

---

### Test 20: Persistence Across Sessions

**Objective**: Verify high score persists across game sessions

**Steps**:
1. **Session 1**:
   - Play game, achieve 1000 points
   - Trigger game over → High Score: 1000
   - Close Unreal Editor completely

2. **Session 2**:
   - Reopen Unreal Editor
   - Start game
   - Achieve 500 points (less than previous high)
   - Trigger game over

**Expected Results**:
- ✅ Session 2 Game Over shows: High Score: 1000 (persisted)
- ✅ Final Score: 500 (current session)
- ✅ High score NOT highlighted (didn't beat record)

**Note**: High score persists in `USideRunnerGameInstance` during editor session. For true cross-session persistence, need to implement `USaveGame` system (future feature).

---

## Regression Testing

### Test 21: Existing Features Still Work

**Objective**: Verify Phase 4 changes didn't break Phase 1-3 features

**Checklist**:

**Phase 1 (Character Movement)**:
- [ ] Player can run forward
- [ ] Jump mechanic works
- [ ] Double jump works
- [ ] Character animations play correctly

**Phase 2 (Health System)**:
- [ ] Health bar displays
- [ ] Taking damage decreases health
- [ ] Health bar color changes (green → yellow → red)
- [ ] Death triggers when health = 0

**Phase 3 (Lives System)**:
- [ ] Lives counter displays
- [ ] Lives decrement on death
- [ ] Respawn works when lives > 0
- [ ] Health restores to full on respawn

**Phase 4 (This Implementation)**:
- [ ] Game HUD appears on start
- [ ] Win condition triggers at 5000m
- [ ] Loss condition triggers at lives = 0
- [ ] Game Over screen functional
- [ ] Restart works correctly

---

## Automated Testing (Future Enhancement)

### Automated Test Cases (C++ Automation Framework)

If implementing UE5 Automation Tests, create these test specs:

```cpp
// Example test structure (not implemented yet)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameOverWinConditionTest,
    "ChromaRunner.GameMode.WinCondition",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter)

bool FGameOverWinConditionTest::RunTest(const FString& Parameters)
{
    // Setup: Create game instance and character
    // Action: Set distance to 5000m
    // Assert: OnGameWon delegate fires
    // Assert: bGameEnded = true
    return true;
}
```

**Test Coverage Goals**:
- Win condition at exact threshold (5000m)
- Loss condition when lives = 0
- Duplicate game over prevention
- High score tracking
- Widget lifecycle (creation/destruction)

---

## Bug Reporting Template

If issues are found during testing, report using this template:

```markdown
### Bug Report: [Short Description]

**Severity**: Critical / High / Medium / Low

**Test Case**: Test #X - [Test Name]

**Steps to Reproduce**:
1. Step one
2. Step two
3. Step three

**Expected Result**:
What should happen

**Actual Result**:
What actually happened

**Screenshots/Video**:
[Attach if applicable]

**Output Log**:
```
[Paste relevant log lines]
```

**Environment**:
- Unreal Engine Version: 5.5.x
- Build Configuration: Development / Shipping / Debug
- Platform: Windows / Linux / Mac

**Related Code**:
- File: Source/SideRunner/SideRunnerGameMode.cpp
- Line: 123
```

---

## Acceptance Criteria Summary

✅ **Must Pass**:
- Test 1: Win Condition (5000m)
- Test 2: Loss Condition (Lives = 0)
- Test 3: Restart Functionality
- Test 10: Input Mode Switching
- Test 11: Widget Lifecycle
- Test 21: Regression Testing

✅ **Should Pass**:
- Test 4-9: HUD and Game Over Screen Display
- Test 12: Widget Binding Verification
- Test 13-15: Edge Cases
- Test 18-19: Integration Tests

✅ **Nice to Have**:
- Test 16-17: Performance Testing
- Test 20: Persistence Testing

---

## Next Steps After Testing

1. **Document Test Results**: Create test report with pass/fail for each test
2. **Fix Critical Bugs**: Address any test failures before proceeding
3. **Polish**: Add animations, particles, sound effects (if time permits)
4. **Create Pull Request**: Merge `hyperthink/phase4-gamemode-ui-system` → `main`
5. **Plan Phase 5**: Determine next feature set (e.g., audio, polish, additional gameplay)

---

**Document Version**: 1.0
**Last Updated**: 2025-11-15
**Author**: ChromaRunner Development Team
**Related**: `Blueprint_UMG_Setup_Guide.md`, Phase 4 Implementation
