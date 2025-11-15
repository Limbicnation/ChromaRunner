# ChromaRunner - Blueprint UMG Widget Setup Guide

This guide provides step-by-step instructions for creating the Blueprint UMG widgets for the Game Over system.

## Prerequisites

✅ C++ classes compiled successfully:
- `ASideRunnerGameMode` - Game mode with widget lifecycle management
- `UGameHUDWidget` - C++ base class for in-game HUD
- `UGameOverWidget` - C++ base class for game over screen

## Part 1: Create WBP_GameHUD (In-Game HUD)

### Step 1: Create the Widget Blueprint

1. Open Unreal Editor
2. Navigate to `Content/SideRunner/Blueprints/` (or your preferred UI folder)
3. Right-click → **User Interface** → **Widget Blueprint**
4. **Parent Class**: Search and select `GameHUDWidget` (C++ class)
5. Name: `WBP_GameHUD`
6. Double-click to open the widget editor

### Step 2: Design the HUD Layout

Add the following UI structure in the **Designer** tab:

```
Canvas Panel (root)
└─ Horizontal Box "TopBar" (Anchor: Top, Fill X)
   ├─ Text Block "LivesText"
   ├─ Spacer (Size: Auto)
   ├─ Text Block "DistanceText"
   ├─ Spacer (Size: Auto)
   └─ Text Block "ScoreText"

Canvas Panel (root)
└─ Text Block "LivesText" (Anchor: Top-Left, Position: X=20, Y=20)
   Text: "Lives: 3/3"
   Font Size: 24
   Color: White
   Shadow: Offset(1,1), Opacity 0.5

Canvas Panel (root)
└─ Text Block "DistanceText" (Anchor: Top-Center)
   Text: "Distance: 0 m"
   Font Size: 48
   Color: White (#FFFFFF)
   Outline: Black, Size 2
   Shadow: Offset(2,2), Opacity 0.8

Canvas Panel (root)
└─ Text Block "ScoreText" (Anchor: Top-Right, Position: X=-20, Y=20)
   Text: "SCORE: 0"
   Font Size: 36
   Color: Cyan (#00E5FF)
   Shadow: Offset(1,1), Opacity 0.5
```

### Step 3: Configure Widget Bindings

**CRITICAL**: Widget variable names MUST match C++ exactly:

1. Select "LivesText" TextBlock → Details panel → **Is Variable**: ✓ Checked
   - Variable Name: `LivesText` (exactly as shown)

2. Select "DistanceText" TextBlock → Details panel → **Is Variable**: ✓ Checked
   - Variable Name: `DistanceText` (exactly as shown)

3. Select "ScoreText" TextBlock → Details panel → **Is Variable**: ✓ Checked
   - Variable Name: `ScoreText` (exactly as shown)

### Step 4: Compile and Save

1. Click **Compile** button (top toolbar)
2. Fix any errors (usually missing bindings)
3. Click **Save**
4. Close widget editor

---

## Part 2: Create WBP_GameOver (Game Over Screen)

### Step 1: Create the Widget Blueprint

1. Navigate to `Content/SideRunner/Blueprints/`
2. Right-click → **User Interface** → **Widget Blueprint**
3. **Parent Class**: Search and select `GameOverWidget` (C++ class)
4. Name: `WBP_GameOver`
5. Double-click to open

### Step 2: Design the Game Over Screen Layout

Add this UI structure in the **Designer** tab:

```
Canvas Panel (root)
└─ Vertical Box "MainContainer" (Anchor: Center, Alignment: 0.5, 0.5)
   ├─ Text Block "GameOverText"
   │  Text: "GAME OVER"
   │  Font Size: 72
   │  Color: Red (#E53935)
   │  Justification: Center
   │  Shadow: Heavy (Offset 3,3, Opacity 1.0)
   │
   ├─ Spacer (Size: 20)
   │
   ├─ Text Block "ScoreText"
   │  Text: "Final Score: 0"
   │  Font Size: 48
   │  Color: White (#FFFFFF)
   │  Justification: Center
   │
   ├─ Text Block "DistanceText"
   │  Text: "Distance: 0 m"
   │  Font Size: 36
   │  Color: Light Gray (#CCCCCC)
   │  Justification: Center
   │
   ├─ Text Block "HighScoreText"
   │  Text: "High Score: 0"
   │  Font Size: 36
   │  Color: White (#FFFFFF)
   │  Justification: Center
   │
   ├─ Text Block "LivesText"
   │  Text: "Lives Used: 0"
   │  Font Size: 28
   │  Color: Gray (#AAAAAA)
   │  Justification: Center
   │
   ├─ Spacer (Size: 40)
   │
   ├─ Button "RestartButton"
   │  ├─ Text Block (child) "Restart Label"
   │  │  Text: "RESTART"
   │  │  Font Size: 32
   │  │  Color: White
   │  │  Justification: Center
   │  Size: 200×60
   │  Normal Color: Green (#4CAF50)
   │  Hovered Color: Bright Green (#66BB6A)
   │  Pressed Color: Dark Green (#2E7D32)
   │
   └─ Button "QuitButton"
      └─ Text Block (child) "Quit Label"
         Text: "QUIT"
         Font Size: 32
         Color: White
         Justification: Center
      Size: 200×60
      Normal Color: Gray (#757575)
      Hovered Color: Light Gray (#9E9E9E)
      Pressed Color: Dark Gray (#424242)
```

### Step 3: Configure Widget Bindings

**CRITICAL**: All widget variable names MUST match C++ exactly:

1. **GameOverText** → Is Variable: ✓, Name: `GameOverText`
2. **ScoreText** → Is Variable: ✓, Name: `ScoreText`
3. **DistanceText** → Is Variable: ✓, Name: `DistanceText`
4. **HighScoreText** → Is Variable: ✓, Name: `HighScoreText`
5. **LivesText** → Is Variable: ✓, Name: `LivesText`
6. **RestartButton** → Is Variable: ✓, Name: `RestartButton`
7. **QuitButton** → Is Variable: ✓, Name: `QuitButton`

### Step 4: Add Background (Optional Polish)

For visual polish, add a semi-transparent background:

1. Add **Image** as first child of Canvas Panel
2. Set to fullscreen (Anchor: Fill, Offset: 0,0,0,0)
3. Set **Color and Opacity**: Black with Alpha 0.8
4. Set **Z-Order**: -1 (behind all other elements)

### Step 5: Compile and Save

1. Click **Compile** (fix any errors)
2. Click **Save**
3. Close widget editor

---

## Part 3: Create/Configure BP_SideRunnerGameMode

### Option A: Reparent Existing GameMode (Recommended)

If you already have a GameMode Blueprint (`BP_SideRunnerGameMode` or `SideRunnerGameMode`):

1. Locate your existing GameMode Blueprint in Content Browser
2. Open the Blueprint
3. Go to **File** → **Reparent Blueprint**
4. Search for and select: `SideRunnerGameMode` (C++ class)
5. Click **OK** to reparent

### Option B: Create New GameMode Blueprint

If no GameMode Blueprint exists:

1. Navigate to `Content/SideRunner/Framework/`
2. Right-click → **Blueprint Class**
3. **Parent Class**: Search and select `SideRunnerGameMode` (C++ class)
4. Name: `BP_SideRunnerGameMode`
5. Double-click to open

### Configure Widget Classes

1. Open `BP_SideRunnerGameMode`
2. In **Class Defaults** panel (right side):
   - Find **UI** category
   - **Game HUD Widget Class**: Click dropdown → Select `WBP_GameHUD`
   - **Game Over Widget Class**: Click dropdown → Select `WBP_GameOver`
3. **Compile** and **Save**

### Set as Default GameMode

1. Open **Edit** → **Project Settings**
2. Navigate to **Maps & Modes** section
3. Under **Default Modes**:
   - **Default GameMode**: Select `BP_SideRunnerGameMode`
4. Close Project Settings

---

## Part 4: Verification Checklist

### Pre-Build Checks

- [ ] WBP_GameHUD created with parent class `GameHUDWidget`
- [ ] WBP_GameHUD has widget bindings: `LivesText`, `DistanceText`, `ScoreText`
- [ ] WBP_GameHUD compiles without errors
- [ ] WBP_GameOver created with parent class `GameOverWidget`
- [ ] WBP_GameOver has widget bindings: `GameOverText`, `ScoreText`, `DistanceText`, `HighScoreText`, `LivesText`, `RestartButton`, `QuitButton`
- [ ] WBP_GameOver compiles without errors
- [ ] BP_SideRunnerGameMode exists and uses C++ parent class
- [ ] BP_SideRunnerGameMode has widget classes assigned
- [ ] BP_SideRunnerGameMode set as default GameMode in Project Settings

### Build and Test

1. Close Unreal Editor (important for C++ recompilation)
2. Run build command:
   ```bash
   make SideRunnerEditor
   ```
3. Wait for successful compilation
4. Reopen Unreal Editor
5. Open your test level (e.g., `TheGame` or `Level_1`)
6. Click **Play** (PIE)

### Expected Behavior

✅ **Game Start:**
- HUD appears showing Lives: 3/3, Distance: 0m, Score: 0
- No errors in Output Log

✅ **During Gameplay:**
- Distance counter increments as player moves forward
- Score increases (distance + coins)
- Lives display updates when player takes damage

✅ **Testing Win Condition:**
- Press **`** (tilde) to open console
- Type: `TeleportToDistance 5000`
- Press Enter
- **Expected**: Game Over screen appears with "YOU WIN!" in green
- Screen shows final stats, high score, Restart/Quit buttons

✅ **Testing Loss Condition:**
- Press **`** (tilde) to open console
- Type: `KillPlayer` (repeat 3 times to exhaust lives)
- **Expected**: Game Over screen appears with "GAME OVER" in red
- Screen shows final stats, Restart/Quit buttons

✅ **Restart Flow:**
- Click "RESTART" button on Game Over screen
- **Expected**: Level reloads, Lives reset to 3/3, Score/Distance reset to 0

---

## Troubleshooting

### Issue: Widget bindings not found (compile errors)

**Solution**: Ensure widget variable names EXACTLY match C++ class member names (case-sensitive):
- `LivesText` ✓ (not `livesText` ✗ or `Lives_Text` ✗)
- `GameOverText` ✓ (not `gameovertext` ✗)

### Issue: Game Over screen doesn't appear

**Checks**:
1. Open Output Log (Window → Developer Tools → Output Log)
2. Search for: `"SideRunnerGameMode: Delegates bound to GameInstance"`
3. If missing, GameMode may not be active. Check Project Settings → Default GameMode
4. Try console command: `TeleportToDistance 5000` to trigger win manually

### Issue: HUD doesn't update

**Checks**:
1. Verify `BP_SideRunnerGameMode` is the active GameMode (check World Settings in level)
2. Check Output Log for: `"SideRunnerGameMode: GameHUD created and added to viewport"`
3. Ensure `GameHUDWidgetClass` is set to `WBP_GameHUD` in GameMode Blueprint

### Issue: Buttons don't respond

**Checks**:
1. Ensure buttons are marked as **Interactive** (Behavior → Is Enabled: ✓)
2. Check button size is adequate (minimum 60×40 pixels)
3. Verify mouse cursor is visible (GameMode should set `bShowMouseCursor = true`)
4. Check Output Log for button click messages

### Issue: Compilation fails with "missing include"

**Solution**: The SideRunner.Build.cs already includes UMG, Slate, and SlateCore modules. Ensure you've saved all files and:
```bash
make clean
make SideRunnerEditor
```

---

## Advanced Customization

### Adding Animations (Optional)

1. Open `WBP_GameOver` in Widget Editor
2. Click **+ Animation** (bottom panel)
3. Name: `FadeIn`
4. Add tracks for GameOverText, ScoreText, DistanceText
5. Keyframe Render Opacity: 0 → 1 over 0.5 seconds
6. In **Graph** tab, add to **Event Construct**:
   ```blueprint
   Event Construct → Play Animation (FadeIn)
   ```

### Color Schemes

**Win State** (modify in SetupGameOverDisplay C++ logic):
- GameOverText: Green (#4CAF50)
- Background particles: Gold/Cyan confetti

**Loss State** (default):
- GameOverText: Red (#E53935)
- Motivational messaging: Yellow (#FFEB3B)

### Responsive Design

For ultra-wide monitors (21:9):
1. Select all text elements
2. Set **Size to Content**: ✓
3. Use **Alignment** instead of fixed positions
4. Test in different viewport sizes (Viewport Options → Common Resolutions)

---

## Next Steps

After completing this guide:

1. ✅ Test win condition: `TeleportToDistance 5000`
2. ✅ Test loss condition: `KillPlayer` (×3)
3. ✅ Test restart flow: Click Restart button
4. ✅ Verify HUD updates during gameplay
5. 📋 Read `Testing_Guide.md` for comprehensive test scenarios
6. 🎨 Customize visuals (colors, fonts, animations) to match your game's theme
7. 🔊 Add UI sounds (button clicks, victory fanfare, defeat music)

---

## Reference

**C++ Classes**:
- `ASideRunnerGameMode` - Source/SideRunner/SideRunnerGameMode.h/.cpp
- `UGameHUDWidget` - Source/SideRunner/GameHUDWidget.h/.cpp
- `UGameOverWidget` - Source/SideRunner/GameOverWidget.h/.cpp
- `USideRunnerGameInstance` - Source/SideRunner/SideRunnerGameInstance.h/.cpp

**Console Commands** (Development builds only):
- `TeleportToDistance <meters>` - Teleport to specific distance
- `KillPlayer` - Instantly kill player
- `stat fps` - Show FPS counter
- `showdebug` - Show debug info

**Key Delegate Events**:
- `OnGameWon` - Fired when player reaches 5000m
- `OnGameLost` - Fired when lives = 0
- `OnScoreUpdated` - Fired when score changes
- `OnDistanceUpdated` - Fired when distance changes
- `OnLivesUpdated` - Fired when lives change

---

**Document Version**: 1.0
**Last Updated**: 2025-11-15
**Author**: ChromaRunner Development Team
