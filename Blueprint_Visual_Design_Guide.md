# Blueprint UMG Widget - Visual Design Guide

**How to Design the Blueprint Widgets in Unreal Editor**

This guide provides detailed visual layout instructions for creating WBP_GameHUD and WBP_GameOver in the Unreal Engine UMG Designer.

---

## 🎨 WBP_GameHUD - In-Game HUD Design

### Layout Preview

```
┌────────────────────────────────────────────────────────────────┐
│ ❤❤❤ 3/3        [Distance: 2,487 m]        SCORE: 8,340      │
│ (Lives)         (Center, Large)             (Top-Right)       │
│                                                                │
│                                                                │
│                   [GAMEPLAY AREA]                              │
│                  Player runs here ──►                          │
│                                                                │
│                                                                │
│                                                                │
│                                                                │
└────────────────────────────────────────────────────────────────┘
```

### Step-by-Step Design in UMG Designer

#### 1. Create Widget Blueprint

1. In Content Browser, navigate to `Content/SideRunner/Blueprints/`
2. Right-click → **User Interface** → **Widget Blueprint**
3. **Parent Class**: Search and select `GameHUDWidget`
4. Name: `WBP_GameHUD`
5. Double-click to open in Designer

---

#### 2. Root Canvas Panel Setup

The root should already be a **Canvas Panel**. If not:
- In **Hierarchy** panel (left), ensure **CanvasPanel** is the root
- This allows precise positioning of UI elements

---

#### 3. Add Lives Display (Top-Left)

**Add Text Block**:
1. In **Palette** panel (top-left), find **Text** under **Common**
2. Drag **Text** onto **Canvas Panel** in Hierarchy
3. Rename to: `LivesText` (right-click → Rename)

**Configure Position** (Details panel, right side):
1. **Anchors**: Click the Anchor dropdown
   - Select: **Top-Left** (icon in top-left corner)
2. **Position**:
   - Position X: `20`
   - Position Y: `20`
3. **Size**:
   - Size X: `150` (or leave as auto)
   - Size Y: `30` (or leave as auto)

**Style the Text**:
1. **Content** section:
   - Text: `Lives: 3/3`
2. **Appearance** section:
   - Font → Size: `24`
   - Color and Opacity: **White** (R:1, G:1, B:1, A:1)
3. **Shadow** (optional for readability):
   - Shadow Offset: X:`1`, Y:`1`
   - Shadow Color: Black with Opacity: `0.5`

**Make it a Variable** (CRITICAL):
1. In Details panel, find **Variable** section (usually at top)
2. Check ✓ **Is Variable**
3. Variable Name should be: `LivesText` (exactly as shown)

---

#### 4. Add Distance Counter (Top-Center)

**Add Text Block**:
1. Drag **Text** from Palette onto Canvas Panel
2. Rename to: `DistanceText`

**Configure Position**:
1. **Anchors**: **Top-Center**
   - This keeps it centered horizontally
2. **Alignment**:
   - Alignment X: `0.5` (centers the text itself)
   - Alignment Y: `0.0`
3. **Position**:
   - Position X: `0` (because we're using center anchor)
   - Position Y: `20`
4. **Size to Content**: ✓ Checked (auto-sizes to text)

**Style the Text**:
1. **Content**:
   - Text: `Distance: 0 m`
2. **Appearance**:
   - Font → Size: `48` (large, very visible)
   - Color: **White** (R:1, G:1, B:1, A:1)
   - Justification: **Center**
3. **Outline** (makes it stand out):
   - In **Appearance**, expand **Outline Settings**
   - Outline Color: **Black** (R:0, G:0, B:0, A:1)
   - Outline Size: `2`
4. **Shadow**:
   - Shadow Offset: X:`2`, Y:`2`
   - Shadow Color: Black, Opacity: `0.8`

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `DistanceText`

---

#### 5. Add Score Display (Top-Right)

**Add Text Block**:
1. Drag **Text** from Palette onto Canvas Panel
2. Rename to: `ScoreText`

**Configure Position**:
1. **Anchors**: **Top-Right**
2. **Alignment**:
   - Alignment X: `1.0` (right-align the text)
   - Alignment Y: `0.0`
3. **Position**:
   - Position X: `-20` (negative for offset from right edge)
   - Position Y: `20`

**Style the Text**:
1. **Content**:
   - Text: `SCORE: 0`
2. **Appearance**:
   - Font → Size: `36`
   - Color: **Cyan** (R:0, G:0.9, B:1, A:1) or use hex #00E5FF
   - Justification: **Right**
3. **Shadow**:
   - Shadow Offset: X:`1`, Y:`1`
   - Shadow Color: Black, Opacity: `0.5`

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `ScoreText`

---

#### 6. Final HUD Verification

**Hierarchy should look like**:
```
CanvasPanel
├─ LivesText (Top-Left, White, 24pt)
├─ DistanceText (Top-Center, White, 48pt, Outline)
└─ ScoreText (Top-Right, Cyan, 36pt)
```

**All 3 text blocks must be marked as Variables** with exact names!

**Compile** → **Save**

---

### Visual Design Tips for HUD

**Readability During Gameplay**:
- Use **outlines** or **shadows** on all text (ensures readability on any background)
- Large font for distance (most important info)
- Contrasting colors (Cyan for score stands out)

**Safe Zones**:
- Keep HUD elements ≥ 20px from screen edges
- Top-center area is safest (no gameplay obstruction)
- Avoid center-left (where player character is)

**Color Psychology**:
- White: Neutral, clear
- Cyan: Attention-grabbing (for score)
- Lives will turn red in C++ when low (automatic)

---

## 🎮 WBP_GameOver - Game Over Screen Design

### Layout Preview (Win State)

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│                      ★ VICTORY! ★                           │
│                   (Gold/Green, 72pt)                        │
│                                                             │
│                  ┌─────────────────┐                        │
│                  │  FINAL SCORE    │                        │
│                  │     12,540      │                        │
│                  └─────────────────┘                        │
│                                                             │
│         ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ 5,000m / 5,000m                │
│                                                             │
│                Distance: 5,000 m                            │
│                High Score: 11,230                           │
│                Lives Used: 1                                │
│                                                             │
│                                                             │
│              ╔═══════════════╗                              │
│              ║    RESTART    ║                              │
│              ╚═══════════════╝                              │
│                                                             │
│              ┌──────────────┐                               │
│              │     QUIT     │                               │
│              └──────────────┘                               │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Step-by-Step Design in UMG Designer

#### 1. Create Widget Blueprint

1. Right-click in Content Browser → **User Interface** → **Widget Blueprint**
2. **Parent Class**: `GameOverWidget`
3. Name: `WBP_GameOver`
4. Open in Designer

---

#### 2. Add Semi-Transparent Background (Optional)

For better readability over gameplay background:

**Add Image**:
1. Drag **Image** from Palette onto Canvas Panel
2. Rename to: `Background` (optional, doesn't need to be a variable)

**Configure**:
1. **Anchors**: **Fill** (stretches to fullscreen)
2. **Offsets**: All `0` (Left, Top, Right, Bottom)
3. **Appearance**:
   - Color and Opacity: **Black** (R:0, G:0, B:0, A:`0.8`)
   - Brush → Tiling: **No Texture** (solid color)

**Z-Order**:
- In Hierarchy, ensure Background is **first** (bottom layer)
- All other elements will appear on top

---

#### 3. Create Main Container (Vertical Box)

**Add Vertical Box**:
1. Drag **Vertical Box** from Palette onto Canvas Panel
2. Rename to: `MainContainer`

**Configure Position**:
1. **Anchors**: **Center** (click the centered diamond icon)
2. **Alignment**: X:`0.5`, Y:`0.5` (centers content)
3. **Position**: X:`0`, Y:`0` (no offset needed when centered)
4. **Size**: Leave as auto or set Width: `600`, Height: `800`

**All subsequent elements go INSIDE this Vertical Box**

---

#### 4. Add Game Over Text (Header)

**Add Text Block** (drag onto **MainContainer** in Hierarchy):
1. Rename to: `GameOverText`

**Configure**:
1. **Slot (Vertical Box Slot)** section:
   - Padding: Top:`40`, Bottom:`20` (spacing around this element)
   - Horizontal Alignment: **Center**
2. **Content**:
   - Text: `GAME OVER`
3. **Appearance**:
   - Font → Size: `72` (very large)
   - Color: **Red** (R:0.9, G:0.22, B:0.22, A:1) - hex #E53935
   - Justification: **Center**
4. **Shadow**:
   - Shadow Offset: X:`3`, Y:`3`
   - Shadow Color: Black, Opacity: `1.0` (heavy shadow for drama)

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `GameOverText`

---

#### 5. Add Spacer (Optional)

**Add Spacer** (drag onto MainContainer):
1. Rename: `Spacer1`
2. **Size**:
   - Set Size: `Override`
   - Size: `20` (pixels of vertical space)

---

#### 6. Add Score Text

**Add Text Block** (onto MainContainer):
1. Rename to: `ScoreText`

**Configure**:
1. **Slot**:
   - Padding: Top:`10`, Bottom:`5`
   - Horizontal Alignment: **Center**
2. **Content**:
   - Text: `Final Score: 0`
3. **Appearance**:
   - Font → Size: `48`
   - Color: **White**
   - Justification: **Center**

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `ScoreText`

---

#### 7. Add Distance Text

**Add Text Block** (onto MainContainer):
1. Rename to: `DistanceText`

**Configure**:
1. **Slot**:
   - Padding: Top:`5`, Bottom:`5`
   - Horizontal Alignment: **Center**
2. **Content**:
   - Text: `Distance: 0 m`
3. **Appearance**:
   - Font → Size: `36`
   - Color: **Light Gray** (R:0.8, G:0.8, B:0.8, A:1)
   - Justification: **Center**

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `DistanceText`

---

#### 8. Add High Score Text

**Add Text Block** (onto MainContainer):
1. Rename to: `HighScoreText`

**Configure**:
1. **Slot**:
   - Padding: Top:`5`, Bottom:`5`
   - Horizontal Alignment: **Center**
2. **Content**:
   - Text: `High Score: 0`
3. **Appearance**:
   - Font → Size: `36`
   - Color: **White**
   - Justification: **Center**

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `HighScoreText`

**Note**: C++ will change this to gold (#FFD700) if new high score

---

#### 9. Add Lives Text

**Add Text Block** (onto MainContainer):
1. Rename to: `LivesText`

**Configure**:
1. **Slot**:
   - Padding: Top:`5`, Bottom:`40` (larger bottom for spacing before buttons)
   - Horizontal Alignment: **Center**
2. **Content**:
   - Text: `Lives Used: 0`
3. **Appearance**:
   - Font → Size: `28`
   - Color: **Gray** (R:0.67, G:0.67, B:0.67, A:1)
   - Justification: **Center**

**Make it a Variable**:
- ✓ **Is Variable**
- Name: `LivesText`

---

#### 10. Add Restart Button

**Add Button** (onto MainContainer):
1. Rename to: `RestartButton`

**Configure Button**:
1. **Slot**:
   - Padding: Top:`10`, Bottom:`10`
   - Horizontal Alignment: **Center**
2. **Behavior**:
   - Is Enabled: ✓ Checked
3. **Style** → Normal:
   - Tint: **Green** (R:0.3, G:0.69, B:0.31, A:1) - hex #4CAF50
4. **Style** → Hovered:
   - Tint: **Bright Green** (R:0.4, G:0.73, B:0.42, A:1) - hex #66BB6A
5. **Style** → Pressed:
   - Tint: **Dark Green** (R:0.18, G:0.49, B:0.2, A:1) - hex #2E7D32

**Configure Button Size**:
1. **Slot** section:
   - Override Size: ✓ Checked
   - Size X: `200`
   - Size Y: `60`

**Add Text Label to Button**:
1. Expand **RestartButton** in Hierarchy
2. It should have a child Text Block (or add one: drag Text onto Button)
3. Configure the child Text:
   - Content → Text: `RESTART`
   - Appearance → Font Size: `32`
   - Appearance → Color: **White**
   - Appearance → Justification: **Center**

**Make Button a Variable**:
- ✓ **Is Variable**
- Name: `RestartButton`

---

#### 11. Add Quit Button

**Add Button** (onto MainContainer):
1. Rename to: `QuitButton`

**Configure Button**:
1. **Slot**:
   - Padding: Top:`10`, Bottom:`40`
   - Horizontal Alignment: **Center**
2. **Behavior**:
   - Is Enabled: ✓ Checked
3. **Style** → Normal:
   - Tint: **Gray** (R:0.46, G:0.46, B:0.46, A:1) - hex #757575
4. **Style** → Hovered:
   - Tint: **Light Gray** (R:0.62, G:0.62, B:0.62, A:1) - hex #9E9E9E
5. **Style** → Pressed:
   - Tint: **Dark Gray** (R:0.26, G:0.26, B:0.26, A:1) - hex #424242

**Configure Button Size**:
1. **Slot** section:
   - Override Size: ✓ Checked
   - Size X: `200`
   - Size Y: `60`

**Add Text Label to Button**:
1. Expand **QuitButton** in Hierarchy
2. Add or configure child Text:
   - Content → Text: `QUIT`
   - Appearance → Font Size: `32`
   - Appearance → Color: **White**
   - Appearance → Justification: **Center**

**Make Button a Variable**:
- ✓ **Is Variable**
- Name: `QuitButton`

---

#### 12. Final Game Over Widget Verification

**Hierarchy should look like**:
```
CanvasPanel
├─ Background (Image, optional)
└─ MainContainer (Vertical Box)
   ├─ GameOverText (72pt, Red)
   ├─ Spacer1
   ├─ ScoreText (48pt, White)
   ├─ DistanceText (36pt, Light Gray)
   ├─ HighScoreText (36pt, White)
   ├─ LivesText (28pt, Gray)
   ├─ RestartButton (200×60, Green)
   │  └─ Text: "RESTART"
   └─ QuitButton (200×60, Gray)
      └─ Text: "QUIT"
```

**All 7 required elements must be Variables**:
- [ ] GameOverText
- [ ] ScoreText
- [ ] DistanceText
- [ ] HighScoreText
- [ ] LivesText
- [ ] RestartButton
- [ ] QuitButton

**Compile** → **Save**

---

## 🎨 Advanced Styling Tips

### Using the Appearance Panel

**Font Settings**:
1. Click on any Text element
2. In Details panel, find **Appearance** → **Font**
3. **Typeface**: Click dropdown to choose font family
   - Default: Roboto (clean, modern)
   - For game feel: Consider bold/condensed fonts
4. **Size**: Adjust for hierarchy (larger = more important)
5. **Outline**: Thickness can be adjusted (0-5 typically)

**Color Picker**:
- Click the color box → Opens color picker
- **Hex Input**: Bottom of picker, can paste hex codes like `#4CAF50`
- **RGB Sliders**: Precise control
- **Eyedropper**: Sample colors from screen

---

### Responsive Design (Multi-Resolution Support)

**Anchors**:
- **Top-Left**: Element stays in top-left corner on any screen size
- **Top-Center**: Element stays centered horizontally
- **Center**: Element stays in exact center of screen
- **Fill**: Element stretches to fill available space

**Alignment**:
- Controls where element's **pivot point** is
- Alignment (0.5, 0.5) = center of element
- Alignment (0, 0) = top-left of element
- Alignment (1, 1) = bottom-right of element

**Size to Content**:
- Check this for text that might change length
- Element auto-resizes to fit content
- Good for score displays that change from "0" to "99,999"

---

### Button States

**Normal**: Default appearance
**Hovered**: Mouse is over button (should be slightly brighter)
**Pressed**: Mouse clicked and held (should be slightly darker)

**Pro Tip**: Hovered should be lighter, Pressed should be darker than Normal
- Creates visual feedback that feels natural
- Example:
  - Normal: Green (#4CAF50)
  - Hovered: Bright Green (#66BB6A) - lighter
  - Pressed: Dark Green (#2E7D32) - darker

---

### Padding & Spacing

**Padding** (in Vertical Box Slot):
- **Top**: Space above this element
- **Bottom**: Space below this element
- **Left**: Space to left (if using Horizontal Box)
- **Right**: Space to right (if using Horizontal Box)

**Good Spacing**:
- Between elements: 10-20px
- Around buttons: 20-40px
- Screen edges: 20px minimum
- Headers: 40px top padding (breathing room)

---

## 📐 Layout Precision

### Snap to Grid

In UMG Designer toolbar:
1. Enable **Grid Snapping** (icon looks like a grid)
2. Set Grid Size: `10` or `20` (aligns elements cleanly)
3. Drag elements - they'll snap to grid positions

### Alignment Tools

Select multiple elements (Ctrl+Click):
- **Align Left**: All elements align to leftmost element
- **Align Center**: All elements center horizontally
- **Align Right**: All elements align to rightmost element
- **Distribute Horizontally**: Equal spacing between elements

---

## 🎯 Quick Design Checklist

### Before Compiling WBP_GameHUD

- [ ] 3 Text Blocks added (LivesText, DistanceText, ScoreText)
- [ ] All 3 have "Is Variable" checked
- [ ] LivesText: Top-Left anchor, White, 24pt
- [ ] DistanceText: Top-Center anchor, White, 48pt, Outline
- [ ] ScoreText: Top-Right anchor, Cyan, 36pt
- [ ] All text has shadows for readability

### Before Compiling WBP_GameOver

- [ ] 7 elements marked as Variables
- [ ] GameOverText: 72pt, Red (or Green for win - C++ changes this)
- [ ] ScoreText: 48pt, White
- [ ] DistanceText: 36pt, Light Gray
- [ ] HighScoreText: 36pt, White
- [ ] LivesText: 28pt, Gray
- [ ] RestartButton: 200×60, Green, "RESTART" text
- [ ] QuitButton: 200×60, Gray, "QUIT" text
- [ ] All elements in Vertical Box, centered
- [ ] Good spacing (padding) between elements

---

## 🐛 Common Design Mistakes

### Mistake 1: Forgot to make element a Variable

**Symptom**: C++ can't find widget binding
**Fix**: Select element → Details → ✓ Is Variable

### Mistake 2: Wrong variable name

**Symptom**: Compile error "LivesText not found"
**Fix**: Names are **case-sensitive**: `LivesText` not `livestext` or `Lives_Text`

### Mistake 3: Element not visible

**Possible causes**:
- Z-order wrong (element behind background)
- Opacity set to 0
- Size too small (check Size X/Y)
- Off-screen position (check Position X/Y)

**Fix**: In Hierarchy, drag element to correct layer, check Visibility and Opacity

### Mistake 4: Button doesn't respond

**Possible causes**:
- "Is Enabled" not checked
- Button too small to click
- Another element covering button

**Fix**:
- Behavior → Is Enabled: ✓
- Minimum button size: 60×40px
- Check Z-order in Hierarchy

### Mistake 5: Text too small to read

**Fix**: Minimum font sizes:
- HUD: 24pt minimum
- Scores: 36-48pt
- Headers: 60-72pt

---

## 📱 Preview Your Design

### Preview in Designer

1. In UMG Designer toolbar, find **Screen Size** dropdown
2. Try different resolutions:
   - 1920×1080 (Full HD)
   - 1280×720 (HD)
   - 2560×1440 (2K)
3. Verify UI scales correctly

### Test in Play Mode

1. **Compile** and **Save** widget
2. **Play (PIE)** to see widget in action
3. For WBP_GameHUD: Should appear automatically
4. For WBP_GameOver: Use console command `TeleportToDistance 5000`

---

## 🎨 Color Reference

### HUD Colors

| Element | Color Name | Hex Code | RGB |
|---------|------------|----------|-----|
| Lives (Normal) | White | #FFFFFF | 1.0, 1.0, 1.0 |
| Lives (Warning) | Yellow | #FFEB3B | 1.0, 0.92, 0.23 |
| Lives (Critical) | Red | #F44336 | 0.96, 0.26, 0.21 |
| Distance | White | #FFFFFF | 1.0, 1.0, 1.0 |
| Score | Cyan | #00E5FF | 0.0, 0.9, 1.0 |

### Game Over Colors (Loss State)

| Element | Color Name | Hex Code | RGB |
|---------|------------|----------|-----|
| Header | Red | #E53935 | 0.9, 0.22, 0.22 |
| Score | White | #FFFFFF | 1.0, 1.0, 1.0 |
| Stats | Light Gray | #CCCCCC | 0.8, 0.8, 0.8 |
| Secondary | Gray | #AAAAAA | 0.67, 0.67, 0.67 |
| Restart Button | Green | #4CAF50 | 0.3, 0.69, 0.31 |
| Quit Button | Gray | #757575 | 0.46, 0.46, 0.46 |

### Game Over Colors (Win State)

| Element | Color Name | Hex Code | RGB |
|---------|------------|----------|-----|
| Header | Green | #4CAF50 | 0.3, 0.69, 0.31 |
| High Score | Gold | #FFD700 | 1.0, 0.84, 0.0 |
| Restart Button | Green | #4CAF50 | 0.3, 0.69, 0.31 |

---

## 🏁 You're Ready!

Once you've designed both widgets following this guide:

1. **Compile** both WBP_GameHUD and WBP_GameOver
2. **Save** both widgets
3. Configure **BP_SideRunnerGameMode** to use these widgets
4. **Test** with console commands:
   - `TeleportToDistance 5000` → Win screen
   - `KillPlayer` (×3) → Loss screen

**Next**: Follow `Game_Over_UMG_System_Plan.md` for testing checklist

---

**Document Version**: 1.0
**Created**: 2025-11-15
**Author**: ChromaRunner Development Team
**Related**: `Blueprint_UMG_Setup_Guide.md`, `Game_Over_UMG_System_Plan.md`
