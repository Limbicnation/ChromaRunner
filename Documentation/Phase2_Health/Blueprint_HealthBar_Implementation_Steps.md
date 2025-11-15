# Blueprint Health Bar Widget - Implementation Workshop

**Project:** ChromaRunner (UE 5.5)
**C++ Class:** `UHealthBarWidget`
**Target Blueprint:** `WBP_HealthBar`
**Location:** `Content/Blueprints/UI/`

This guide provides step-by-step instructions for creating the Widget Blueprint that connects to the C++ UHealthBarWidget class. Follow each section in order while working in Unreal Editor.

---

## Prerequisites Checklist

Before starting, verify these conditions:

- [ ] Project compiles successfully (Ctrl+Alt+F11 or "Live Coding" in editor)
- [ ] `HealthBarWidget.h` and `HealthBarWidget.cpp` exist in `Source/SideRunner/`
- [ ] `PlayerHealthComponent` is attached to `BP_RunnerCharacter`
- [ ] Unreal Editor is open with ChromaRunner project loaded

---

## Part 1: Creating the Widget Blueprint

### Step 1.1: Navigate to UI Folder

1. In **Content Browser**, navigate to: `Content/Blueprints/`
2. If `UI` folder doesn't exist, create it:
   - Right-click in Content Browser → **New Folder**
   - Name: `UI`
   - Press Enter

### Step 1.2: Create Widget Blueprint

1. **Right-click** inside `Content/Blueprints/UI/` folder
2. Hover over **User Interface** in context menu
3. Click **Widget Blueprint**
4. In the "Pick Parent Class" window:
   - **IMPORTANT:** Do NOT select "User Widget"
   - Use the search box at top-right
   - Type: `HealthBarWidget`
   - Click on **HealthBarWidget** (should show "SideRunner" module)
   - Click **Select** button

### Step 1.3: Name the Blueprint

1. New blueprint asset appears with default name selected
2. Type exactly: `WBP_HealthBar`
3. Press **Enter** to confirm

### Step 1.4: Open the Widget Blueprint

1. **Double-click** `WBP_HealthBar` to open Widget Designer
2. You should see:
   - **Designer** tab (visual editor)
   - **Graph** tab (Blueprint graph)
   - **Palette** panel (left side - widget types)
   - **Hierarchy** panel (top-left - widget tree)
   - **Details** panel (right side - properties)

**Checkpoint:** Your Widget Blueprint should now be open with a blank canvas and "CanvasPanel" as root.

---

## Part 2: Designer Layout - Adding Required Widgets

The C++ class expects **exact widget names** for automatic binding. Follow naming precisely.

### Step 2.1: Set Up Root Container

1. In **Hierarchy** panel, you should see **CanvasPanel**
2. If not, add one:
   - In **Palette** panel, search: `Canvas Panel`
   - Drag **Canvas Panel** to Hierarchy as root
3. Select **CanvasPanel** in Hierarchy
4. In **Details** panel:
   - **Is Variable:** Unchecked (we don't need a reference)

### Step 2.2: Add Progress Bar Widget (REQUIRED)

**CRITICAL:** This widget MUST be named exactly `HealthProgressBar` for C++ binding to work.

1. In **Palette** panel, search: `Progress Bar`
2. Drag **Progress Bar** onto the canvas or into **Hierarchy** under CanvasPanel
3. **IMMEDIATELY RENAME** (before doing anything else):
   - In **Hierarchy**, the widget appears as "ProgressBar_0" or similar
   - **Right-click** on it → **Rename**
   - Type exactly: `HealthProgressBar`
   - Press **Enter**

4. With `HealthProgressBar` selected, configure in **Details** panel:

#### Slot (Canvas Panel Slot) Section:
```
Position X: 50.0
Position Y: 50.0
Size X: 400.0
Size Y: 30.0
Anchors: Top-Left (preset dropdown, top-left corner icon)
Alignment X: 0.0
Alignment Y: 0.0
Z-Order: 1
```

#### Appearance Section:
```
Fill Type: Left to Right
Percent: 1.0 (will be controlled by C++)
Fill Color and Opacity: RGB(0, 255, 0, 255) - Green (will be controlled by C++)
Background Image: Default (or choose a texture if desired)
```

#### Style Section (optional customization):
```
Border Padding: Margin(0, 0, 0, 0)
```

#### Behavior Section:
```
Is Variable: Checked ✓ (REQUIRED for C++ binding)
```

### Step 2.3: Add Text Block Widget (OPTIONAL)

This widget displays the hit counter. It's optional but recommended.

1. In **Palette** panel, search: `Text Block`
2. Drag **Text Block** onto the canvas or into **Hierarchy** under CanvasPanel
3. **IMMEDIATELY RENAME**:
   - Right-click → **Rename**
   - Type exactly: `HitCounterText`
   - Press **Enter**

4. With `HitCounterText` selected, configure in **Details** panel:

#### Slot (Canvas Panel Slot) Section:
```
Position X: 50.0
Position Y: 90.0
Size X: 200.0
Size Y: 30.0
Anchors: Top-Left
Alignment X: 0.0
Alignment Y: 0.0
Z-Order: 2
```

#### Content Section:
```
Text: Hits: 0 (placeholder - will be replaced by C++)
```

#### Appearance Section:
```
Color and Opacity: RGB(255, 255, 255, 255) - White
Font:
  - Font Family: Roboto (or any clear font)
  - Typeface: Regular
  - Size: 18
  - Outline Size: 1.0 (optional, for readability)
  - Outline Color: RGB(0, 0, 0, 255) - Black
```

#### Behavior Section:
```
Is Variable: Checked ✓ (REQUIRED for C++ binding)
```

**Checkpoint:** Your Hierarchy should now look like:
```
CanvasPanel
├─ HealthProgressBar
└─ HitCounterText
```

---

## Part 3: Visual Styling and Polish

### Step 3.1: Customize Progress Bar Appearance (Optional)

With `HealthProgressBar` selected in Hierarchy:

#### Add Background Color:
1. In **Details** → **Appearance** → **Background Image**
2. Click the dropdown next to "Image"
3. Set **Tint** to: RGB(60, 60, 60, 255) - Dark Gray

#### Add Border/Frame:
1. If you have a border texture asset:
   - **Appearance** → **Fill Image** → Select your texture
2. Otherwise, rely on Fill Color (already green)

### Step 3.2: Position Widgets for Screen Visibility

**Option A: Top-Left Corner (Recommended for HUD)**

1. Select `CanvasPanel` in Hierarchy
2. In **Details** → **Slot (Parent Slot)**:
   - If no slot settings, the CanvasPanel is the root (correct)

3. Select `HealthProgressBar`:
   - **Position X:** 20.0
   - **Position Y:** 20.0
   - **Anchors:** Click dropdown → Select **Top-Left** preset
   - **Size to Content:** Unchecked

4. Select `HitCounterText`:
   - **Position X:** 20.0
   - **Position Y:** 60.0
   - **Anchors:** Click dropdown → Select **Top-Left** preset

**Option B: Top-Center (Alternate)**

1. Select `HealthProgressBar`:
   - **Anchors:** Click dropdown → Select **Top-Center** preset
   - **Position X:** -200.0 (half of Size X for centering)
   - **Position Y:** 20.0
   - **Alignment X:** 0.5

2. Select `HitCounterText`:
   - **Anchors:** Click dropdown → Select **Top-Center** preset
   - **Position X:** -100.0 (half of Size X for centering)
   - **Position Y:** 60.0
   - **Alignment X:** 0.5

### Step 3.3: Add Visual Effects (Optional)

**Drop Shadow for Text:**
1. Select `HitCounterText`
2. In **Details** → **Appearance** → **Shadow Offset**:
   - X: 2.0
   - Y: 2.0
3. **Shadow Color and Opacity:** RGB(0, 0, 0, 128) - Semi-transparent black

**Progress Bar Animation Prep:**
1. Select `HealthProgressBar`
2. In **Details** → **Render Transform**:
   - Leave at defaults (animations can be added later)

---

## Part 4: Binding Verification

### Step 4.1: Check Widget Names

**CRITICAL:** C++ binding ONLY works if names match exactly.

1. In **Hierarchy** panel, verify:
   - [ ] `HealthProgressBar` (NOT "HealthBar", "ProgressBar", etc.)
   - [ ] `HitCounterText` (NOT "HitCounter", "TextBlock", etc.)

2. Both widgets must have **Is Variable** checked:
   - Select `HealthProgressBar` → **Details** → **Behavior** → **Is Variable**: ✓
   - Select `HitCounterText` → **Details** → **Behavior** → **Is Variable**: ✓

### Step 4.2: Compile the Widget Blueprint

1. Click **Compile** button (top toolbar, green checkmark icon)
2. Wait for compilation to finish
3. Check **Compiler Results** panel (bottom of screen):
   - **Success:** Green checkmark, no errors
   - **Warnings:** Yellow, may indicate missing bindings
   - **Errors:** Red X, must be fixed before proceeding

**Expected Output:**
```
Success: WidgetBlueprint '/Game/Blueprints/UI/WBP_HealthBar' compiled successfully.
```

### Step 4.3: Verify C++ Binding (Advanced)

1. Click **Graph** tab (top of editor)
2. In **My Blueprint** panel (left side), expand **Variables**
3. You should see:
   - `HealthProgressBar` (type: Progress Bar (Object Reference))
   - `HitCounterText` (type: Text Block (Object Reference))
4. These were automatically created by C++ `BindWidget` meta tags

**If variables don't appear:**
- Return to **Designer** tab
- Verify widget names are EXACT matches
- Ensure **Is Variable** is checked
- Click **Compile** again

### Step 4.4: Test Widget Properties (Optional)

1. Return to **Designer** tab
2. In **Details** panel (top section), you should see C++ properties:
   - **Health Bar** category:
     - `bUseSmoothColorTransition` (default: true)
     - `HealthyColor` (default: Green)
     - `CautionColor` (default: Yellow)
     - `CriticalColor` (default: Red)
     - `HitCounterFormat` (default: "Hits: {0}")

3. These can be customized per instance if needed

---

## Part 5: Adding Widget to Viewport

You have **two options** for displaying the health bar: add it to the character directly, or create a HUD class. Choose the method that fits your project architecture.

### Option A: Add to Character Blueprint (Simple, Recommended)

This method creates the widget when the character spawns.

#### Step 5A.1: Open Character Blueprint

1. In **Content Browser**, navigate to your character blueprint location
2. Open `BP_RunnerCharacter` (or your main character blueprint)

#### Step 5A.2: Add Widget Creation Logic

1. Click **Event Graph** tab
2. Find or create **Event BeginPlay** node:
   - If exists: Use existing
   - If not: Right-click → Search "Event BeginPlay" → Add

3. **Add Create Widget Node:**
   - Drag off **BeginPlay** execution pin (white arrow)
   - Search: `Create Widget`
   - Select **Create Widget** (returns User Widget Object Reference)

4. **Configure Create Widget Node:**
   - **Class** dropdown → Select `WBP_HealthBar`
   - **Owning Player** → Leave as default (uses Player 0)

5. **Add to Viewport:**
   - Drag off **Return Value** pin of Create Widget node
   - Search: `Add to Viewport`
   - Select **Add to Viewport** (Target is User Widget)

6. **Optional: Store Reference:**
   - Drag off **Return Value** pin of Create Widget node
   - Search: `Promote to Variable`
   - Name it: `HealthBarWidget`
   - Connect this variable to Add to Viewport node

#### Step 5A.3: Blueprint Node Configuration

Your Event Graph should look like this:

```
[Event BeginPlay] → [Create Widget]
                      ↓ (Class: WBP_HealthBar)
                      ↓ (Owning Player: Get Player Controller → Player Index 0)
                      ↓
                     [Return Value] → [Add to Viewport]
                                       ↓ (Z-Order: 0)
```

**Node Details:**
- **Create Widget:**
  - Class: `WBP_HealthBar`
  - Owning Player: (auto-connects to self's player controller)

- **Add to Viewport:**
  - Z-Order: `0` (lower = behind other UI, higher = in front)

#### Step 5A.4: Compile Character Blueprint

1. Click **Compile** button
2. Click **Save** button
3. Close character blueprint

---

### Option B: Add to HUD Class (Advanced, More Control)

This method centralizes UI management in a dedicated HUD class.

#### Step 5B.1: Create or Open HUD Blueprint

**If you don't have a HUD class:**
1. In Content Browser → **Blueprints** folder
2. Right-click → **Blueprint Class**
3. Search for: `HUD`
4. Select **HUD** class → **Select**
5. Name it: `BP_GameHUD`

**If you already have a HUD:**
1. Open your existing HUD blueprint

#### Step 5B.2: Add Widget Creation to HUD

1. Open `BP_GameHUD` Event Graph
2. Find **Event BeginPlay** or **Event ReceiveDrawHUD** (first-time only)

3. Add the same Create Widget → Add to Viewport logic as in Option A

#### Step 5B.3: Assign HUD to Game Mode

1. Open your Game Mode blueprint (`BP_SideRunner_GameMode`)
2. In **Details** panel:
   - Find **HUD Class** dropdown
   - Select `BP_GameHUD`
3. **Compile** and **Save**

---

## Part 6: Testing and Verification

### Step 6.1: Pre-Play Checklist

Before testing, verify:

- [ ] `WBP_HealthBar` compiled successfully (no errors)
- [ ] Widget added to viewport in character or HUD blueprint
- [ ] Character blueprint compiled successfully
- [ ] `PlayerHealthComponent` exists on `BP_RunnerCharacter`
- [ ] Project compiles with no C++ errors

### Step 6.2: Play in Editor (PIE)

1. Click **Play** button (Alt+P) in main editor toolbar
2. Observe top-left corner of game viewport

**Expected Result:**
- Green progress bar appears (100% filled)
- "Hits: 0" text appears below it
- No error messages in Output Log

**If health bar doesn't appear:**
- Check **Output Log** (Window → Developer Tools → Output Log)
- Look for errors containing "HealthBarWidget"
- See Troubleshooting section below

### Step 6.3: Test Health Changes

You need a way to damage the character. Choose one:

**Method 1: Blueprint Command (Quick Test)**
1. In `BP_RunnerCharacter` Event Graph
2. Add keyboard event:
   - Right-click → **Keyboard Events** → **H**
3. Drag off **Pressed** pin → Search: `Take Damage`
4. Connect to `PlayerHealthComponent → Take Damage`
   - Damage Amount: `10`
   - Damage Type: `Spikes`
5. Compile, Save, Play
6. Press **H** key during play

**Method 2: Use Existing Spikes**
1. Place spike obstacles in your level
2. Run character into spikes
3. Observe health bar decrease

**Expected Behavior:**
- Health bar fills decreases (100% → 90% → 80%, etc.)
- Bar color changes:
  - **100%-66%:** Green → Yellow transition
  - **66%-33%:** Yellow → Red transition
  - **Below 33%:** Pure red
- Hit counter increments: "Hits: 1", "Hits: 2", etc.

### Step 6.4: Test Death State

Continue taking damage until health reaches 0:

**Expected Result:**
- Health bar shows 0% (empty)
- Bar is red
- Hit counter shows total hits taken
- Character enters death animation (if implemented)

### Step 6.5: Check Output Log

1. Stop PIE (press Escape or Stop button)
2. Open **Output Log** (Window → Developer Tools → Output Log)
3. Search for "HealthBarWidget" messages

**Expected Log Messages (Development build):**
```
LogTemp: Warning: HealthBarWidget: Failed to bind to health component - retrying every 0.1s
LogTemp: Log: HealthBarWidget: Successfully bound to health component
LogTemp: VeryVerbose: HealthBarWidget: Health changed to 100 / 100 (100.0%)
LogTemp: VeryVerbose: HealthBarWidget: Took 10 damage (Type: 0), Total hits: 1
LogTemp: Verbose: HealthBarWidget: Updated bar to 90.0% with color (0.45, 1.00, 0.00)
```

**Warning about retry:** The warning is normal if the widget loads before the character is fully initialized. The retry mechanism handles this automatically.

---

## Part 7: Customization and Polish

### Step 7.1: Adjust Health Bar Colors

1. Open `WBP_HealthBar` in Designer
2. Switch to **Graph** tab
3. In **My Blueprint** → **Variables**, find class defaults
4. Or, place widget instance in level and modify in Details:

**Modify Colors:**
- **HealthyColor:** RGB(0, 255, 0) → Your custom green
- **CautionColor:** RGB(255, 255, 0) → Your custom yellow
- **CriticalColor:** RGB(255, 0, 0) → Your custom red

**Toggle Smooth Transitions:**
- **bUseSmoothColorTransition:** True/False
  - True: Smooth color interpolation (looks better)
  - False: Hard color thresholds (better performance)

### Step 7.2: Customize Hit Counter Format

Change the display format of the hit counter:

**Examples:**
- `"Hits: {0}"` (default)
- `"Damage Taken: {0}x"`
- `"❤ {0}"`
- `"{0} Times Hit"`

Modify in:
- Widget Blueprint instance **Details** → **Health Bar** → **Hit Counter Format**
- Or in C++ default: `HitCounterFormat = FText::FromString(TEXT("Your format"));`

### Step 7.3: Add Background Panel (Optional)

For better visibility, add a background:

1. Open `WBP_HealthBar` in Designer
2. In Palette, search: `Image`
3. Drag **Image** into Hierarchy **ABOVE** HealthProgressBar
4. Rename to: `BackgroundImage`
5. Configure:
   - **Size:** Slightly larger than progress bar (e.g., 410x40)
   - **Position:** Behind progress bar
   - **Color:** RGB(0, 0, 0, 180) - Semi-transparent black
   - **Z-Order:** -1 (behind other elements)

### Step 7.4: Add Animations (Advanced)

Create a damage flash effect:

1. Click **Animations** tab (bottom of Designer)
2. Click **+ Animation** button
3. Name it: `DamageFlash`
4. In timeline:
   - Track 0.0s: `HealthProgressBar` opacity = 1.0
   - Track 0.1s: `HealthProgressBar` opacity = 0.5
   - Track 0.2s: `HealthProgressBar` opacity = 1.0
5. Set animation to play on damage (requires Blueprint graph logic)

---

## Troubleshooting Guide

### Issue 1: Widget Doesn't Appear in Game

**Symptoms:** Playing game, but no health bar visible.

**Checklist:**
- [ ] Did you add widget to viewport in character/HUD blueprint?
- [ ] Is `Create Widget` node's **Class** set to `WBP_HealthBar`?
- [ ] Is **Add to Viewport** node connected?
- [ ] Check Z-Order (should be 0 or positive)
- [ ] Verify character blueprint compiled successfully

**Debug Steps:**
1. Add **Print String** node after Create Widget
   - String: "Health Bar Created"
2. PIE and check if message appears
3. If message appears but no widget:
   - Check widget size/position in Designer
   - Try changing position to center of screen (960, 540)

---

### Issue 2: Binding Errors in Output Log

**Symptoms:** Errors like "HealthProgressBar is not bound!"

**Error Message:**
```
LogTemp: Error: HealthBarWidget: HealthProgressBar is not bound!
Make sure you have a ProgressBar widget named 'HealthProgressBar' in your UMG Designer.
```

**Solution:**
1. Open `WBP_HealthBar` in Designer
2. Select Progress Bar in Hierarchy
3. **Right-click → Rename**
4. Type **EXACTLY:** `HealthProgressBar` (case-sensitive!)
5. Verify **Is Variable** is checked
6. Compile widget
7. Test again

**Common Name Mistakes:**
- ❌ `HealthBar` (missing "Progress")
- ❌ `healthProgressBar` (wrong capitalization)
- ❌ `Health_Progress_Bar` (underscores not allowed)
- ✅ `HealthProgressBar` (CORRECT)

---

### Issue 3: Health Bar Doesn't Update

**Symptoms:** Widget appears, but doesn't change when taking damage.

**Checklist:**
- [ ] Is `PlayerHealthComponent` attached to character?
- [ ] Does component broadcast `OnHealthChanged` delegate?
- [ ] Check Output Log for binding success message

**Debug Steps:**
1. Open `BP_RunnerCharacter`
2. Check **Components** panel (left side)
3. Verify `PlayerHealthComponent` exists
4. If missing, add it:
   - Click **Add Component** → Search "PlayerHealthComponent"
   - Make it public (checkbox)
5. Compile, save, test again

**Advanced Check:**
1. In character Event Graph
2. Get `PlayerHealthComponent` reference
3. Call `GetCurrentHealth`
4. Connect to **Print String**
5. Verify health value is changing when damaged

---

### Issue 4: Widget Appears But Shows Wrong Values

**Symptoms:** Widget visible, but shows 0% health or incorrect hit count.

**Possible Causes:**
- Health component not initialized
- Widget created before character BeginPlay
- Binding retry mechanism still running

**Solution:**
1. Check Output Log for:
   ```
   LogTemp: Log: HealthBarWidget: Successfully bound to health component
   ```
2. If you see continuous retry warnings:
   - Character may not have health component
   - Component may not be public (UPROPERTY)
3. Verify in `RunnerCharacter.h`:
   ```cpp
   UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
   UPlayerHealthComponent* HealthComponent;
   ```

---

### Issue 5: Color Doesn't Change

**Symptoms:** Health decreases, but bar stays green.

**Checklist:**
- [ ] Is `bUseSmoothColorTransition` enabled?
- [ ] Are color values configured correctly?
- [ ] Check if Fill Color in Designer overrides C++

**Solution:**
1. Open `WBP_HealthBar` in Designer
2. Select `HealthProgressBar`
3. In **Details** → **Appearance** → **Fill Color and Opacity**:
   - **IMPORTANT:** Do NOT bind this to a function
   - Leave as default - C++ controls it
4. Verify in **Graph** tab that no color bindings exist
5. Test again

---

### Issue 6: Performance Issues / Lag

**Symptoms:** Game stutters when health bar is visible.

**Optimization Steps:**
1. Open `WBP_HealthBar` in Designer
2. Verify **NativeTick** is NOT overridden in Graph
3. Check **Class Settings**:
   - **Tick Frequency:** Never (should be default)
4. Disable smooth color transition:
   - **bUseSmoothColorTransition:** False
5. Reduce widget complexity:
   - Remove unnecessary visual effects
   - Use simpler fonts
   - Avoid heavy materials on progress bar

---

### Issue 7: Widget Appears Behind Other UI

**Symptoms:** Health bar is drawn behind pause menu or other widgets.

**Solution:**
1. Open character/HUD blueprint
2. Find **Add to Viewport** node
3. Set **Z-Order** to higher value:
   - HUD elements: 0
   - Health bar: 10
   - Pause menu: 100
4. Higher Z-Order = drawn in front

---

## Advanced Topics

### Topic 1: Multiple Health Bars (Enemies, Bosses)

To show health bars for enemies:

1. Create new widget `WBP_EnemyHealthBar` (parent: `UHealthBarWidget`)
2. In enemy blueprint's Event Graph:
   - Create widget on spawn
   - **Add to Viewport** (Z-Order: 5)
   - Position above enemy (use "Add to Player Screen" with world position)
3. Bind to enemy's health component

**Note:** C++ class supports any actor with `UPlayerHealthComponent` (despite name).

---

### Topic 2: Animating Health Changes

Add smooth fill animation:

1. Enable **NativeTick** override in C++ (performance cost)
2. Interpolate `CurrentHealth` to `TargetHealth` using `FMath::FInterpTo`
3. Or use UMG animations (no C++ changes needed)

---

### Topic 3: Damage Flash Effect

Integrate damage flash with C++ event:

1. In `WBP_HealthBar` Graph:
2. Override **OnTakeDamage** event (if exposed to Blueprint)
3. Or add Blueprint-callable function in C++:
   ```cpp
   UFUNCTION(BlueprintImplementableEvent)
   void PlayDamageFlash();
   ```
4. Call from `OnTakeDamage` C++ function
5. Implement in Blueprint Graph with animation playback

---

### Topic 4: Responsive UI Scaling

Make widget scale with resolution:

1. Select `CanvasPanel` in Hierarchy
2. Replace with **Scale Box**:
   - Right-click CanvasPanel → Replace With → Scale Box
3. Configure Scale Box:
   - **Stretch:** Scale to Fit
   - **Stretch Direction:** Both
4. Add CanvasPanel inside Scale Box
5. Set anchors on children for responsive positioning

---

## Quick Reference

### Required Widget Names (Case-Sensitive!)

| C++ Property | Widget Type | Required Name | Required? |
|--------------|-------------|---------------|-----------|
| HealthProgressBar | Progress Bar | `HealthProgressBar` | Yes ✓ |
| HitCounterText | Text Block | `HitCounterText` | No (optional) |

### Default Color Values

| Property | RGB Value | Hex | Usage |
|----------|-----------|-----|-------|
| HealthyColor | (0, 255, 0) | #00FF00 | >66% health |
| CautionColor | (255, 255, 0) | #FFFF00 | 33-66% health |
| CriticalColor | (255, 0, 0) | #FF0000 | <33% health |

### Blueprint Nodes for Viewport Addition

```
Event BeginPlay
  → Create Widget (Class: WBP_HealthBar, Owning Player: Player Controller)
    → Add to Viewport (Z-Order: 0)
```

### Common Console Commands for Testing

| Command | Function |
|---------|----------|
| `stat fps` | Show frame rate |
| `stat unit` | Show frame time breakdown |
| `showdebug` | Toggle debug display |
| `debughealth` | (Custom - if implemented) |

---

## Completion Checklist

You've successfully implemented the health bar widget when:

- [ ] `WBP_HealthBar` exists in `Content/Blueprints/UI/`
- [ ] Widget parent class is `UHealthBarWidget`
- [ ] `HealthProgressBar` widget added with exact name
- [ ] `HitCounterText` widget added with exact name
- [ ] Both widgets have "Is Variable" checked
- [ ] Widget compiles with no errors
- [ ] Widget added to viewport in character/HUD blueprint
- [ ] Health bar appears in PIE test
- [ ] Health bar updates when taking damage
- [ ] Colors transition green → yellow → red
- [ ] Hit counter increments on damage
- [ ] No errors in Output Log after binding succeeds

---

## Additional Resources

### UE5 Documentation Links

- **UMG Widget Designer:** https://docs.unrealengine.com/5.5/en-US/umg-ui-designer-quick-start-guide/
- **Progress Bar Widget:** https://docs.unrealengine.com/5.5/en-US/API/Runtime/UMG/Components/UProgressBar/
- **C++ UMG Binding:** https://docs.unrealengine.com/5.5/en-US/using-cplusplus-umg-widgets/

### Project Files Reference

- C++ Header: `Source/SideRunner/HealthBarWidget.h`
- C++ Implementation: `Source/SideRunner/HealthBarWidget.cpp`
- Health Component: `Source/SideRunner/PlayerHealthComponent.h`
- Character Blueprint: `Content/Blueprints/BP_RunnerCharacter.uasset`

---

## Workflow Summary

For quick future reference:

1. **Create:** User Interface → Widget Blueprint → Parent: `HealthBarWidget`
2. **Name:** `WBP_HealthBar`
3. **Add Widgets:**
   - Progress Bar → Rename to `HealthProgressBar` → Is Variable ✓
   - Text Block → Rename to `HitCounterText` → Is Variable ✓
4. **Style:** Position, size, colors, fonts as desired
5. **Compile:** Green checkmark, verify no errors
6. **Add to Viewport:** Character/HUD → Create Widget → Add to Viewport
7. **Test:** PIE → Take damage → Verify updates

---

**End of Workshop Guide**

*Last Updated: 2025-11-13*
*ChromaRunner Project - UE 5.5*
*C++ Health Bar Widget Implementation*
