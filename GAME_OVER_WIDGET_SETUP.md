# Game Over Widget Setup Guide

This guide provides step-by-step instructions for setting up the WBP_GameOver Blueprint widget in Unreal Engine 5.5.

## Problem

The game over flow is working correctly in C++, but the WBP_GameOver widget is empty, causing an invisible screen. This guide will help you add all required UI elements.

## Required Elements

The GameOverWidget C++ class expects the following named elements:

### Text Blocks (5 total)
- **GameOverText** - Main "GAME OVER" or "YOU WIN!" message
- **ScoreText** - Display final score
- **DistanceText** - Display distance traveled
- **HighScoreText** - Display high score
- **LivesText** - Display lives used

### Buttons (2 total)
- **RestartButton** - Restart the level
- **QuitButton** - Quit the game

## Step-by-Step Setup

### 1. Open WBP_GameOver Widget

1. In Unreal Editor, navigate to `Content/Blueprints/UI/` (or wherever WBP_GameOver is located)
2. Double-click **WBP_GameOver** to open the UMG Designer

### 2. Add Canvas Panel (if not present)

1. If the widget is completely empty, add a **Canvas Panel** as the root:
   - In the Palette panel, search for "Canvas Panel"
   - Drag it onto the Hierarchy
   - This will be the root container for all UI elements

### 3. Add Background (Optional but Recommended)

1. Add an **Image** widget to darken the background:
   - Drag **Image** from Palette to Canvas Panel
   - Rename it to "Background"
   - In Details panel:
     - Anchors: Set to **Fill** (stretches to fullscreen)
     - Color and Opacity: Set to black with alpha 0.7 (semi-transparent)
     - ZOrder: 0 (behind everything)

### 4. Add Vertical Box for Layout

1. Add a **Vertical Box** to organize elements vertically:
   - Drag **Vertical Box** from Palette to Canvas Panel
   - Rename it to "MainLayout"
   - In Details panel:
     - Anchors: Set to **Center** (middle of screen)
     - Position X: 0, Position Y: 0
     - Alignment: 0.5, 0.5 (centered)
     - Size to Content: true

### 5. Add GameOverText (Main Title)

1. Drag **Text Block** from Palette into **MainLayout** Vertical Box
2. Rename it to **GameOverText** (EXACT NAME - case sensitive!)
3. In Details panel:
   - Text: "GAME OVER" (placeholder)
   - Font Size: 72 (large and visible)
   - Justification: Center
   - Color: Red (will be set dynamically by C++)
4. In Slot settings (in parent Vertical Box):
   - Padding: Top 50, Bottom 30
   - Horizontal Alignment: Center

### 6. Add ScoreText

1. Drag another **Text Block** into **MainLayout**
2. Rename it to **ScoreText**
3. In Details panel:
   - Text: "Final Score: 0" (placeholder)
   - Font Size: 36
   - Justification: Center
4. In Slot settings:
   - Padding: Top 10, Bottom 10
   - Horizontal Alignment: Center

### 7. Add DistanceText

1. Drag **Text Block** into **MainLayout**
2. Rename it to **DistanceText**
3. In Details panel:
   - Text: "Distance: 0.0 m" (placeholder)
   - Font Size: 36
   - Justification: Center
4. In Slot settings:
   - Padding: Top 10, Bottom 10
   - Horizontal Alignment: Center

### 8. Add HighScoreText

1. Drag **Text Block** into **MainLayout**
2. Rename it to **HighScoreText**
3. In Details panel:
   - Text: "High Score: 0" (placeholder)
   - Font Size: 36
   - Justification: Center
4. In Slot settings:
   - Padding: Top 10, Bottom 10
   - Horizontal Alignment: Center

### 9. Add LivesText

1. Drag **Text Block** into **MainLayout**
2. Rename it to **LivesText**
3. In Details panel:
   - Text: "Lives Used: 0" (placeholder)
   - Font Size: 36
   - Justification: Center
4. In Slot settings:
   - Padding: Top 10, Bottom 30
   - Horizontal Alignment: Center

### 10. Add RestartButton

1. Drag **Button** from Palette into **MainLayout**
2. Rename it to **RestartButton** (EXACT NAME!)
3. Add a **Text Block** as a child of the button:
   - Drag **Text Block** onto **RestartButton** in hierarchy
   - Set text to "RESTART"
   - Font Size: 32
   - Justification: Center
4. Configure button in Details panel:
   - Style → Normal → Tint: Dark blue
   - Style → Hovered → Tint: Light blue
   - Style → Pressed → Tint: Green
5. In Slot settings:
   - Padding: Top 20, Bottom 10
   - Horizontal Alignment: Center

### 11. Add QuitButton

1. Drag **Button** from Palette into **MainLayout**
2. Rename it to **QuitButton** (EXACT NAME!)
3. Add a **Text Block** as a child of the button:
   - Set text to "QUIT"
   - Font Size: 32
   - Justification: Center
4. Configure button in Details panel:
   - Style → Normal → Tint: Dark red
   - Style → Hovered → Tint: Light red
   - Style → Pressed → Tint: Gray
5. In Slot settings:
   - Padding: Top 10, Bottom 20
   - Horizontal Alignment: Center

### 12. Bind Variables to C++

**CRITICAL STEP - DO NOT SKIP!**

For each UI element created above, you MUST bind it to the C++ variable:

1. Select **GameOverText** in the hierarchy
2. In Details panel, find "Is Variable" checkbox
3. **CHECK** the "Is Variable" box
4. The variable name MUST match exactly: `GameOverText`

Repeat for all elements:
- ✅ GameOverText
- ✅ ScoreText
- ✅ DistanceText
- ✅ HighScoreText
- ✅ LivesText
- ✅ RestartButton
- ✅ QuitButton

### 13. Set Parent Class

1. In the **Graph** view (click "Graph" tab at top right)
2. Click "Class Settings" in the toolbar
3. In Details panel → Class Options:
   - **Parent Class**: Should be set to **GameOverWidget** (the C++ class)
4. If not set, select GameOverWidget from the dropdown

### 14. Compile and Save

1. Click **Compile** button (top left)
2. Fix any errors that appear
3. Click **Save** button
4. Close the UMG Designer

### 15. Assign Widget to Game Mode

1. Open **BP_SideRunnerGameMode** (in Content/Blueprints/)
2. In Details panel, find **Game Over Widget Class**
3. Set it to **WBP_GameOver**
4. Compile and save

## Verification

Run the game and trigger game over (die 3 times or use console command `TriggerGameOver`).

You should see in the log:

```
LogTemp: Warning: === GameOverWidget Binding Validation ===
LogTemp:   ✓ GameOverText found
LogTemp:   ✓ ScoreText found
LogTemp:   ✓ DistanceText found
LogTemp:   ✓ HighScoreText found
LogTemp:   ✓ LivesText found
LogTemp:   ✓ RestartButton found and bound
LogTemp:   ✓ QuitButton found and bound
LogTemp: === All widget bindings valid ✓ ===
```

If you see ❌ errors, double-check the exact naming and variable binding.

## Quick Visual Reference

Your hierarchy should look like this:

```
WBP_GameOver (Canvas Panel)
├── Background (Image) [OPTIONAL]
└── MainLayout (Vertical Box)
    ├── GameOverText (Text Block) ✓
    ├── ScoreText (Text Block) ✓
    ├── DistanceText (Text Block) ✓
    ├── HighScoreText (Text Block) ✓
    ├── LivesText (Text Block) ✓
    ├── RestartButton (Button) ✓
    │   └── ButtonText (Text Block)
    └── QuitButton (Button) ✓
        └── ButtonText (Text Block)
```

## Troubleshooting

### "Widget elements are NULL" errors

**Cause**: Variable binding not set or name mismatch

**Solution**:
1. Select each UI element
2. Check "Is Variable" checkbox in Details panel
3. Verify the name matches EXACTLY (case-sensitive):
   - `GameOverText` not `gameOverText` or `Game_Over_Text`

### Widget appears but text is empty

**Cause**: C++ class not set as parent

**Solution**:
1. Open Widget Blueprint → Graph view
2. Class Settings → Parent Class → GameOverWidget
3. Compile and save

### Buttons don't work

**Cause**: Button variable binding missing or name wrong

**Solution**:
1. Verify button names are EXACTLY `RestartButton` and `QuitButton`
2. Check "Is Variable" checkbox for both buttons
3. Recompile widget

### Game tears down immediately after game over

**Cause**: Game pause not working (already fixed in C++)

**Solution**: Rebuild the C++ project - game pause code has been added

## Need Help?

Check the Output Log for detailed diagnostic messages. The GameOverWidget will tell you exactly which elements are missing!

---

**Last Updated**: 2025-11-17
**Engine Version**: Unreal Engine 5.5
**Module**: ChromaRunner / SideRunner
