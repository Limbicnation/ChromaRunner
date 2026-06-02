#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GroundPatrol.generated.h"

// ─── Patrol Direction Enum ─────────────────────────────────────────────────────

/** Direction the patrolling entity is currently moving. */
UENUM(BlueprintType)
enum class EPatrolDirection : int8
{
	Forward  =  1  UMETA(DisplayName = "Forward"),
	Backward = -1  UMETA(DisplayName = "Backward")
};

// ─── Patrol State Enum ────────────────────────────────────────────────────────

/** High-level state of the patrol behaviour. */
UENUM(BlueprintType)
enum class EPatrolState : uint8
{
	Idle              UMETA(DisplayName = "Idle"),
	Moving            UMETA(DisplayName = "Moving"),
	PausedAtBoundary  UMETA(DisplayName = "Paused at Boundary")
};

// ─── Configuration Struct ──────────────────────────────────────────────────────

/** Data-driven config for UGroundPatrolComponent. Editable per-instance. */
USTRUCT(BlueprintType)
struct FGroundPatrolConfig
{
	GENERATED_BODY()

	/** Movement speed in Unreal units per second. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "10.0", ClampMax = "2000.0"))
	float Speed = 200.0f;

	/** Maximum distance the entity patrols from its spawn point (each direction). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
	float PatrolDistance = 400.0f;

	/** Length of the downward line trace used to detect ledges. 0 = disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Detection", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float LedgeDetectionDistance = 100.0f;

	/** Length of the forward line trace used to detect walls. 0 = disabled. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Detection", meta = (ClampMin = "0.0", ClampMax = "500.0"))
	float WallDetectionDistance = 60.0f;

	/** How long to pause (seconds) at each patrol boundary before reversing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol", meta = (ClampMin = "0.0", ClampMax = "10.0"))
	float PauseDuration = 0.0f;

	/** Enable ledge detection (downward raycast at leading edge). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Detection")
	bool bEnableLedgeDetection = true;

	/** Enable wall detection (forward raycast). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol|Detection")
	bool bEnableWallDetection = true;

	/** Axis along which the entity patrols. Default is Y for a 2.5D side-scroller. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FVector MovementAxis = FVector(0.0f, 1.0f, 0.0f);
};

// ─── Delegate Declarations ─────────────────────────────────────────────────────

/** Fires when the patrol direction changes (Forward ↔ Backward). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatrolDirectionChanged, EPatrolDirection, NewDirection);

/** Fires when the patrol state changes (Idle ↔ Moving ↔ Paused). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPatrolStateChanged, EPatrolState, NewState);

// ─── UGroundPatrolComponent ────────────────────────────────────────────────────

/**
 * Reusable ActorComponent that drives back-and-forth ground patrol movement.
 *
 * Architecture:
 *   - Tick-driven, frame-rate independent via DeltaTime.
 *   - Reverses direction on boundary distance, wall hits, or ledge detection.
 *   - Broadcasts delegates for direction / state changes (no visual coupling).
 *   - Configured via FGroundPatrolConfig (data-driven, editor-exposed).
 *
 * Usage:
 *   1. Add UGroundPatrolComponent to any AActor (e.g. an enemy character).
 *   2. Configure FGroundPatrolConfig in the Details panel.
 *   3. Optionally bind to FOnPatrolDirectionChanged for sprite flipping.
 *   4. Call SetPatrolEnabled(true) to begin patrolling.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class SIDERUNNER_API UGroundPatrolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGroundPatrolComponent();

	// ── Configuration ───────────────────────────────────────────────────────

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
	FGroundPatrolConfig PatrolConfig;

	// ── Delegates ───────────────────────────────────────────────────────────

	UPROPERTY(BlueprintAssignable, Category = "Patrol|Events")
	FOnPatrolDirectionChanged OnPatrolDirectionChanged;

	UPROPERTY(BlueprintAssignable, Category = "Patrol|Events")
	FOnPatrolStateChanged OnPatrolStateChanged;

	// ── Public API ──────────────────────────────────────────────────────────

	/** Start or stop patrolling. */
	UFUNCTION(BlueprintCallable, Category = "Patrol")
	void SetPatrolEnabled(bool bEnabled);

	/** Get the current patrol direction. */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	EPatrolDirection GetPatrolDirection() const { return Direction; }

	/** Get the current patrol state. */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	EPatrolState GetPatrolState() const { return State; }

	/** Get the origin position where patrol started. */
	UFUNCTION(BlueprintPure, Category = "Patrol")
	FVector GetPatrolOrigin() const { return PatrolOrigin; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	// ── Internal State ──────────────────────────────────────────────────────

	FVector PatrolOrigin;
	EPatrolDirection Direction = EPatrolDirection::Forward;
	EPatrolState State = EPatrolState::Idle;
	bool bPatrolEnabled = false;
	float PauseTimer = 0.0f;

	// ── Core Movement ───────────────────────────────────────────────────────

	void UpdateMovement(float DeltaTime);
	void MoveInDirection(float DeltaTime);
	bool ShouldReverseAtBoundary() const;
	bool DetectWall() const;
	bool DetectLedge() const;
	void ReverseDirection();
	void SetState(EPatrolState NewState);
};
