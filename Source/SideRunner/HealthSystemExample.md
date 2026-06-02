# Health System Architecture — Integration Guide

## Class Diagram

```
┌─────────────────────────────────────────────────────────────────┐
│                        AActor (e.g. Enemy)                      │
│  ┌─────────────────────────┐                                    │
│  │ UGroundPatrolComponent  │──→ FOnPatrolDirectionChanged       │
│  │  FGroundPatrolConfig    │──→ FOnPatrolStateChanged           │
│  │  EPatrolDirection       │                                    │
│  │  EPatrolState           │                                    │
│  └─────────────────────────┘                                    │
└─────────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────────┐
│                     ARunnerCharacter (Player)                    │
│  ┌──────────────────────┐                                       │
│  │   UPlayerHealth*     │──→ FOnHealthChanged(float, float)     │
│  │  FPlayerHealthConfig │──→ FOnDeath()                         │
│  │  EPlayerHealthState  │──→ FOnTakeDamage(int32, EDamageType)  │
│  │  EDamageType         │──→ FOnPlayerDeath(int32)              │
│  └──────────────────────┘──→ FOnInvulnerabilityStarted()        │
│                             └→ FOnInvulnerabilityEnded()        │
└─────────────────────────────────────────────────────────────────┘
          │ delegates
          ▼
┌─────────────────────────────────────────────────────────────────┐
│                     UHealthBarUI (Widget)                        │
│  ┌──────────────────────────────────────────────────────────┐   │
│  │  TWeakObjectPtr<UPlayerHealth> ObservedHealth            │   │
│  │  float NormalizedHealth (0.0 – 1.0)                      │   │
│  │  UProgressBar* HealthProgressBar  (BindWidget)           │   │
│  └──────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

## How the Classes Interact

### Observer Pattern (Decoupling)

`UPlayerHealth` has **no knowledge** of `UHealthBarUI`. Communication is one-way via UE delegates:

1. **Player takes damage** → `UPlayerHealth::TakeDamage()` → fires `OnHealthChanged(CurrentHealth, MaxHealth)`
2. **UHealthBarUI** has bound to `OnHealthChanged` in `NativeConstruct()` → callback updates `NormalizedHealth`
3. **UHealthBarUI** updates `UProgressBar::SetPercent(NormalizedHealth)`

### Memory Safety

- `UHealthBarUI` holds a `TWeakObjectPtr<UPlayerHealth>` (engine smart pointer), not a raw pointer.
- If the player actor is destroyed, `ObservedHealth.IsValid()` returns false and the widget safely does nothing.
- All delegate bindings are removed in `NativeDestruct()` to prevent dangling callbacks.

### Config-Driven Initialization

Both `UPlayerHealth` and `UGroundPatrolComponent` expose `USTRUCT` config objects:

```cpp
// In Blueprint Details panel or C++ constructor:
HealthComponent->Config.MaxHealth = 5.0f;
HealthComponent->Config.InvulnerabilityDuration = 2.0f;

PatrolComponent->PatrolConfig.Speed = 150.0f;
PatrolComponent->PatrolConfig.PatrolDistance = 300.0f;
PatrolComponent->PatrolConfig.bEnableLedgeDetection = true;
```

---

## Initialization Examples

### Player Character (ARunnerCharacter)

```cpp
// In ARunnerCharacter constructor:
HealthComponent = CreateDefaultSubobject<UPlayerHealth>(TEXT("HealthComponent"));

// In BeginPlay:
HealthComponent->OnHealthChanged.AddDynamic(this, &ARunnerCharacter::OnHealthChanged);
HealthComponent->OnPlayerDeath.AddDynamic(this, &ARunnerCharacter::HandlePlayerDeath);
```

### Enemy Actor with Patrol

```cpp
// In AMyEnemy constructor:
PatrolComponent = CreateDefaultSubobject<UGroundPatrolComponent>(TEXT("PatrolComponent"));

// In BeginPlay:
PatrolComponent->PatrolConfig.Speed = 200.0f;
PatrolComponent->PatrolConfig.PatrolDistance = 400.0f;
PatrolComponent->PatrolConfig.MovementAxis = FVector(0.0f, 1.0f, 0.0f); // Y-axis
PatrolComponent->SetPatrolEnabled(true);
```

### Health Bar Widget

```cpp
// In UHealthBarUI::NativeConstruct (automatic):
// 1. Finds owning player's ARunnerCharacter
// 2. Gets HealthComponent
// 3. Binds to OnHealthChanged, OnTakeDamage, OnPlayerDeath
// 4. Initializes NormalizedHealth from current state

// The widget updates automatically — no manual polling needed.
```

---

## Game Loop Flow

```
Frame N:
  ├─ UGroundPatrolComponent::TickComponent(dt)
  │   ├─ Check wall/ledge/boundary → may reverse direction
  │   ├─ Move owning actor along MovementAxis
  │   └─ Broadcast FOnPatrolDirectionChanged if reversed
  │
  ├─ ASimpleEnemy::OnOverlapBegin (collision with player)
  │   ├─ Validate player, check cooldown
  │   ├─ UPlayerHealth::TakeDamage(amount, EDamageType::EnemyMelee)
  │   │   ├─ Apply damage, clamp health
  │   │   ├─ Broadcast FOnHealthChanged → UHealthBarUI updates
  │   │   ├─ Broadcast FOnTakeDamage (legacy)
  │   │   ├─ Start invulnerability timer if configured
  │   │   └─ If health ≤ 0 → TriggerDeath()
  │   │       ├─ Broadcast FOnDeath
  │   │       └─ Broadcast FOnPlayerDeath(totalHits)
  │   └─ Set damage cooldown timer
  │
  └─ UHealthBarUI::NativeTick (optional smooth interpolation)
      └─ Lerp visual fill toward NormalizedHealth
```

---

## Migration from Old Classes

| Old | New | Notes |
|-----|-----|-------|
| `UPlayerHealthComponent` | `UPlayerHealth` | Same delegate names, same API surface |
| `UHealthBarWidget` | `UHealthBarUI` | Uses `TWeakObjectPtr` instead of raw `UPROPERTY` |
| `ASimpleEnemy::SimplePatrolMovement()` | `UGroundPatrolComponent::TickComponent()` | Wall/ledge detection added |
| `PlayerHealthComponent.h` | `PlayerHealth.h` | `EDamageType` moved here |
| `HealthBarWidget.h` | `HealthBarUI.h` | Includes `PlayerHealth.h` |

### Blueprint Migration

1. **BP_RunnerCharacter**: Change `HealthComponent` type from `UPlayerHealthComponent` to `UPlayerHealth`.
2. **WBP_GameHUD**: Change parent class of health bar widget from `UHealthBarWidget` to `UHealthBarUI`.
3. **Enemy BPs**: Add `UGroundPatrolComponent`, configure `PatrolConfig`, remove inline patrol logic.
