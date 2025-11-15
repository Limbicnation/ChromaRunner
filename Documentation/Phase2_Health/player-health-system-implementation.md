```markdown
# Jira Task: Player Health System Implementation

## Game Damage System and Health UI Implementation

### Role/Context
You are implementing a comprehensive damage and health system for an Unreal Engine side-runner game. The existing codebase includes damage-dealing obstacles (like the Spikes class), and you need to create a robust health management system with visual feedback for the player character.

---

### Primary Task Description
Create a modular damage reception system for the player character that:
- Manages health state with clear max/current health tracking.
- Processes incoming damage from various sources (obstacles, enemies).
- Displays real-time health status through UI components.
- Tracks cumulative damage statistics for gameplay feedback.
- Includes invulnerability frames to prevent damage spamming.

---

### Core Requirements & Specifications

#### 1. Health System Variables
-   `int32 MaxHealth`: Player's maximum health capacity.
-   `int32 CurrentHealth`: Player's remaining health.
-   `int32 TotalHitsTaken`: Cumulative hit counter, initialized to 0 on spawn.
-   `float InvulnerabilityTime`: Configurable duration (e.g., 1.0f) for invulnerability frames after taking damage.
-   `TArray<int32> DamageValues`: Damage amounts categorized by source type (e.g., Spikes, Enemy Melee, Enemy Projectile, Environmental Hazard).
-   `float LowHealthThreshold`: Health percentage (e.g., 0.25f) to trigger low-health warnings.

#### 2. System Logic & Processing
-   **Initialization:** On player spawn, `CurrentHealth` should be set to `MaxHealth`, and `TotalHitsTaken` reset to 0.
-   **Damage Reception (`TakeDamage` function):**
    -   Validate that the damage amount is greater than 0.
    -   Apply damage: `CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0, MaxHealth)`.
    -   Increment `TotalHitsTaken`.
    -   Trigger invulnerability frames.
    -   Trigger a UI update.
    -   Check for player death (`CurrentHealth <= 0`) and trigger the appropriate game state change.
-   **Fall Damage:** Instant death from fall damage might be too harsh. Consider applying a fixed high damage value (e.g., 50) for environmental hazards instead.
-   **Performance:** Use a timer-based approach instead of the `Tick` function for updating the invulnerability timer to optimize performance.

#### 3. UI Component Design
-   **Health Bar Widget:**
    -   **Dimensions:** 300px width, 30px height.
    -   **Position:** Top-left corner with a 20px margin.
    -   **Fill:** A gradient from Green (`#00FF00`) to Red (`#FF0000`) based on the health percentage.
    -   **Background:** Dark gray (`#333333`) with a 2px border.
    -   **Animation:** Health changes should animate smoothly over 0.3 seconds.
-   **Hit Counter Display:**
    -   **Position:** Below the health bar.
    -   **Format:** "Hits Taken: [TotalHitsTaken]".
    -   **Font:** Bold, 16pt, white text with a black outline.
-   **Damage Feedback:**
    -   The UI should flash a red overlay when damage is taken.
    -   The hit counter should update immediately.

---

### Output & Expected Behaviors

#### Required Components
1.  **PlayerHealthComponent:** A clean, component-based class to manage all health state, damage processing, and death logic.
2.  **HealthBarWidget:** A Blueprint/C++ class for the UI, including the progress bar, hit counter, and animations.
3.  **Delegates:**
    -   `OnTakeDamage`: For damage events.
    -   `OnHealthChanged`: For UI updates.
    -   `OnPlayerDeath`: For game state changes.

#### Expected Functionality
-   Damage reduces health smoothly and does not go below 0.
-   UI updates reflect health changes in near real-time (<100ms).
-   Invulnerability frames prevent rapid, successive hits.
-   Player death triggers the correct game state transitions.

---

### Quality & Implementation Criteria
-   **Architecture:** Maintain a clean, component-based design with a strong separation of concerns (health logic isolated from character logic).
-   **Modularity:** Components should be easily extendable for future features like buffs, debuffs, or healing mechanisms.
-   **Robustness:** The system must handle edge cases like negative damage, overflow, and rapid hits. Use null checks and safety guards.
-   **Configuration:** Avoid hardcoded "magic numbers." Values like invulnerability time and low health thresholds should be configurable `UPROPERTY` variables.
-   **Documentation:** Add comments explaining the damage type system and the event flow (damage -> health component -> UI).

---

### Testing & Validation
The implementation must be tested against the following cases:
-   Verify that running into spikes applies the correct damage.
-   Confirm the health bar updates in real-time.
-   Ensure the player character dies when health reaches 0.
-   Test that invulnerability frames successfully prevent rapid damage.
-   Write unit tests for the following:
    -   Damage application correctly reduces health.
    -   Invulnerability prevents damage.
    -   Death is triggered at 0 health.
    -   Health percentage calculations are accurate.

---

### Missing Features (Future Scope)
The following items are not in the initial scope but should be considered for future iterations:
-   Healing mechanisms.
-   Health persistence between levels.
-   Additional damage feedback (e.g., screen flash, sound effects).
```