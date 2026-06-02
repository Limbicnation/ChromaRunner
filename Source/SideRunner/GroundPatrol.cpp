#include "GroundPatrol.h"
#include "SideRunner.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UGroundPatrolComponent::UGroundPatrolComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UGroundPatrolComponent::BeginPlay()
{
	Super::BeginPlay();

	PatrolOrigin = GetOwner()->GetActorLocation();
	Direction = EPatrolDirection::Forward;
	SetState(EPatrolState::Idle);

	UE_LOG(LogSideRunner, Log,
		TEXT("[Patrol] %s initialized at %s, Speed=%.0f, Dist=%.0f"),
		*GetOwner()->GetName(), *PatrolOrigin.ToString(),
		PatrolConfig.Speed, PatrolConfig.PatrolDistance);
}

void UGroundPatrolComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UGroundPatrolComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bPatrolEnabled)
	{
		UpdateMovement(DeltaTime);
	}
}

// ─── Public API ────────────────────────────────────────────────────────────────

void UGroundPatrolComponent::SetPatrolEnabled(bool bEnabled)
{
	bPatrolEnabled = bEnabled;

	if (bEnabled)
	{
		SetState(EPatrolState::Moving);
	}
	else
	{
		SetState(EPatrolState::Idle);
		PauseTimer = 0.0f;
	}
}

// ─── Core Movement ─────────────────────────────────────────────────────────────

void UGroundPatrolComponent::UpdateMovement(float DeltaTime)
{
	if (State == EPatrolState::PausedAtBoundary)
	{
		PauseTimer -= DeltaTime;
		if (PauseTimer <= 0.0f)
		{
			PauseTimer = 0.0f;
			ReverseDirection();
			SetState(EPatrolState::Moving);
		}
		return;
	}

	// Check reversal triggers before moving
	if (ShouldReverseAtBoundary())
	{
		if (PatrolConfig.PauseDuration > 0.0f)
		{
			PauseTimer = PatrolConfig.PauseDuration;
			SetState(EPatrolState::PausedAtBoundary);
			return;
		}
		ReverseDirection();
	}

	if (PatrolConfig.bEnableWallDetection && DetectWall())
	{
		if (PatrolConfig.PauseDuration > 0.0f)
		{
			PauseTimer = PatrolConfig.PauseDuration;
			SetState(EPatrolState::PausedAtBoundary);
			return;
		}
		ReverseDirection();
	}

	if (PatrolConfig.bEnableLedgeDetection && DetectLedge())
	{
		if (PatrolConfig.PauseDuration > 0.0f)
		{
			PauseTimer = PatrolConfig.PauseDuration;
			SetState(EPatrolState::PausedAtBoundary);
			return;
		}
		ReverseDirection();
	}

	MoveInDirection(DeltaTime);
}

void UGroundPatrolComponent::MoveInDirection(float DeltaTime)
{
	AActor* Owner = GetOwner();
	if (!Owner) return;

	const float DirMultiplier = static_cast<float>(Direction);
	const FVector Movement = PatrolConfig.MovementAxis.GetSafeNormal()
		* DirMultiplier * PatrolConfig.Speed * DeltaTime;

	Owner->SetActorLocation(Owner->GetActorLocation() + Movement);
}

bool UGroundPatrolComponent::ShouldReverseAtBoundary() const
{
	const FVector CurrentPos = GetOwner()->GetActorLocation();
	const FVector Offset = CurrentPos - PatrolOrigin;

	// Project offset onto movement axis to get 1D distance
	const FVector AxisNorm = PatrolConfig.MovementAxis.GetSafeNormal();
	const float DistanceAlongAxis = FVector::DotProduct(Offset, AxisNorm);

	return FMath::Abs(DistanceAlongAxis) >= PatrolConfig.PatrolDistance;
}

bool UGroundPatrolComponent::DetectWall() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return false;

	const UWorld* World = GetWorld();
	if (!World) return false;

	const FVector Start = Owner->GetActorLocation();
	const float DirMultiplier = static_cast<float>(Direction);
	const FVector ForwardDir = PatrolConfig.MovementAxis.GetSafeNormal() * DirMultiplier;
	const FVector End = Start + ForwardDir * PatrolConfig.WallDetectionDistance;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	const bool bHit = World->LineTraceSingleByChannel(
		Hit, Start, End, ECC_WorldStatic, Params);

#if ENABLE_DRAW_DEBUG
	if (bHit)
	{
		DrawDebugLine(World, Start, Hit.ImpactPoint, FColor::Red, false, -1.0f, 0, 2.0f);
	}
#endif

	return bHit;
}

bool UGroundPatrolComponent::DetectLedge() const
{
	const AActor* Owner = GetOwner();
	if (!Owner) return false;

	const UWorld* World = GetWorld();
	if (!World) return false;

	const FVector ActorLoc = Owner->GetActorLocation();
	const float DirMultiplier = static_cast<float>(Direction);
	const FVector ForwardDir = PatrolConfig.MovementAxis.GetSafeNormal() * DirMultiplier;

	// Cast from slightly ahead and above the actor, downward
	const FVector Start = ActorLoc + ForwardDir * 40.0f + FVector(0.0f, 0.0f, 50.0f);
	const FVector End = Start - FVector(0.0f, 0.0f, PatrolConfig.LedgeDetectionDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	const bool bHit = World->LineTraceSingleByChannel(
		Hit, Start, End, ECC_WorldStatic, Params);

#if ENABLE_DRAW_DEBUG
	DrawDebugLine(World, Start, End, bHit ? FColor::Green : FColor::Yellow,
		false, -1.0f, 0, 1.0f);
#endif

	// If we DON'T hit anything, there's a ledge ahead
	return !bHit;
}

void UGroundPatrolComponent::ReverseDirection()
{
	Direction = (Direction == EPatrolDirection::Forward)
		? EPatrolDirection::Backward
		: EPatrolDirection::Forward;

	OnPatrolDirectionChanged.Broadcast(Direction);

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, Verbose, TEXT("[Patrol] %s reversed to %s"),
		*GetOwner()->GetName(),
		Direction == EPatrolDirection::Forward ? TEXT("Forward") : TEXT("Backward"));
#endif
}

void UGroundPatrolComponent::SetState(EPatrolState NewState)
{
	if (State == NewState) return;

	State = NewState;
	OnPatrolStateChanged.Broadcast(NewState);

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogSideRunner, Verbose, TEXT("[Patrol] %s state → %s"),
		*GetOwner()->GetName(), *UEnum::GetValueAsString(NewState));
#endif
}
