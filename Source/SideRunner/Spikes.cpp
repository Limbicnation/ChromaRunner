#include "Spikes.h"
#include "Kismet/GameplayStatics.h"
#include "RunnerCharacter.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DamageEvents.h"

#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif

// PERFORMANCE: Optimized constructor with efficient initialization
ASpikes::ASpikes()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create collision box with optimized settings
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionBox->SetNotifyRigidBodyCollision(true);

	// Create mesh component with optimized collision settings
	SpikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpikeMesh"));
	SpikeMesh->SetupAttachment(CollisionBox);
	SpikeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Collision handled by box

	// Create particle effect component
	ImpactEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ImpactEffect"));
	ImpactEffect->SetupAttachment(RootComponent);
	ImpactEffect->bAutoActivate = false;

	// PERFORMANCE: Initialize with sensible defaults
	Speed = 100.0f;
	MaxMovementOffset = 100.0f;
	MovementDirection = 1;
	DamageAmount = 10.0f;
	DamageCooldown = 1.0f;
	DamageTimer = 0.0f;
	bIsMoving = true;
	MovementType = EMovementType::UpDown;

	// PERFORMANCE: Initialize proximity trigger properties
	bProximityTriggered = false;
	TriggerRadius = 300.0f;
	bIsTriggered = false;
	CurrentTime = 0.0f;
}

void ASpikes::BeginPlay()
{
	Super::BeginPlay();

	// Store initial position for movement calculations
	InitialPosition = GetActorLocation();

	// Optimize particle effect initialization
	if (ImpactEffect)
	{
		ImpactEffect->Deactivate();
	}

	// Reset timers
	DamageTimer = 0.0f;
	CurrentTime = 0.0f;

#if UE_BUILD_DEBUG
	// PERFORMANCE: Only log warnings in debug builds
	if (!CollisionSound)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: CollisionSound is not set in the editor!"), *GetName());
	}
#endif
}

void ASpikes::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// PERFORMANCE: Early exit for immobile spikes
	if (!bIsMoving)
	{
		// Still need to handle damage cooldown even when not moving
		if (DamageTimer > 0.0f)
		{
			DamageTimer -= DeltaTime;
		}
		return;
	}

	// Update damage timer
	if (DamageTimer > 0.0f)
	{
		DamageTimer -= DeltaTime;
	}

	// PERFORMANCE: Handle proximity triggering efficiently
	if (bProximityTriggered)
	{
		static APawn* CachedPlayer = nullptr;
		
		// Cache player reference to avoid repeated GetPlayerCharacter calls
		if (!CachedPlayer || !IsValid(CachedPlayer))
		{
			CachedPlayer = UGameplayStatics::GetPlayerCharacter(this, 0);
		}
		
		if (CachedPlayer)
		{
			// PERFORMANCE: Use squared distance to avoid expensive square root
			const float DistanceSquared = FVector::DistSquared(GetActorLocation(), CachedPlayer->GetActorLocation());
			const float TriggerRadiusSquared = TriggerRadius * TriggerRadius;
			bIsTriggered = (DistanceSquared <= TriggerRadiusSquared);
			
			if (!bIsTriggered)
			{
				return; // Don't move if not triggered
			}
		}
		else
		{
			return; // No player found
		}
	}

	// Update movement time
	CurrentTime += DeltaTime;

	// PERFORMANCE: Calculate movement based on type using optimized math
	FVector NewLocation;
	CalculateMovementLocation(NewLocation, DeltaTime);
	
	SetActorLocation(NewLocation);
}

void ASpikes::CalculateMovementLocation(FVector& OutLocation, float DeltaTime)
{
	OutLocation = GetActorLocation();
	
	// PERFORMANCE: Precompute common values
	const float SpeedFactor = Speed / 100.0f;
	const float SinValue = FMath::Sin(CurrentTime * SpeedFactor);
	const float CosValue = FMath::Cos(CurrentTime * SpeedFactor);
	
	switch (MovementType)
	{
	case EMovementType::UpDown:
		OutLocation.Z = InitialPosition.Z + SinValue * MaxMovementOffset;
		break;

	case EMovementType::LeftRight:
		OutLocation.X = InitialPosition.X + SinValue * MaxMovementOffset;
		break;

	case EMovementType::FrontBack:
		OutLocation.Y = InitialPosition.Y + SinValue * MaxMovementOffset;
		break;

	case EMovementType::Circular:
		OutLocation.X = InitialPosition.X + SinValue * MaxMovementOffset;
		OutLocation.Y = InitialPosition.Y + CosValue * MaxMovementOffset;
		break;

	case EMovementType::Zigzag:
		CalculateZigzagMovement(OutLocation, SpeedFactor);
		break;

	case EMovementType::Static:
	default:
		OutLocation = InitialPosition;
		break;
	}
}

void ASpikes::CalculateZigzagMovement(FVector& OutLocation, float SpeedFactor)
{
	// PERFORMANCE: Optimized zigzag calculation
	const float CyclePosition = FMath::Fmod(CurrentTime * SpeedFactor * 2.0f, 4.0f);
	
	if (CyclePosition < 1.0f)
	{
		OutLocation.X = InitialPosition.X + CyclePosition * MaxMovementOffset;
	}
	else if (CyclePosition < 2.0f)
	{
		OutLocation.X = InitialPosition.X + MaxMovementOffset;
	}
	else if (CyclePosition < 3.0f)
	{
		OutLocation.X = InitialPosition.X + (3.0f - CyclePosition) * MaxMovementOffset;
	}
	else
	{
		OutLocation.X = InitialPosition.X;
	}

	// Add slight vertical movement
	OutLocation.Z = InitialPosition.Z + FMath::Sin(CurrentTime * SpeedFactor) * (MaxMovementOffset * 0.2f);
}

void ASpikes::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp,
	bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// PERFORMANCE: Early type check and null validation
	ARunnerCharacter* PlayerCharacter = Cast<ARunnerCharacter>(Other);
	if (!PlayerCharacter)
	{
		return;
	}

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, Verbose, TEXT("%s: Hit detected with player"), *GetName());
#endif

	// Handle collision effects and damage
	HandleCollisionEffects(HitLocation);
	ApplyDamageToPlayer(PlayerCharacter);
}

void ASpikes::HandleCollisionEffects(const FVector& HitLocation)
{
	// Play collision sound
	if (CollisionSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CollisionSound, GetActorLocation());
	}

	// Show particle effect at collision point
	if (ImpactEffect)
	{
		ImpactEffect->SetWorldLocation(HitLocation);
		ImpactEffect->Activate(true);
	}
}

void ASpikes::SetMovementEnabled(bool bEnabled)
{
	bIsMoving = bEnabled;
	
	// PERFORMANCE: Reset time when enabling/disabling to prevent jarring transitions
	if (bEnabled)
	{
		CurrentTime = 0.0f;
	}
}

void ASpikes::ApplyDamageToPlayer(AActor* Player)
{
	// PERFORMANCE: Check cooldown first to avoid unnecessary work
	if (DamageTimer > 0.0f)
	{
		return;
	}

	// Apply damage to player
	FDamageEvent DamageEvent;
	Player->TakeDamage(DamageAmount, DamageEvent, nullptr, this);

	// Set cooldown timer
	DamageTimer = DamageCooldown;

#if UE_BUILD_DEVELOPMENT
	UE_LOG(LogTemp, Log, TEXT("%s: Applied %.1f damage to %s"), *GetName(), DamageAmount, *Player->GetName());
#endif
}

#if WITH_EDITOR
void ASpikes::DrawDebugMovementPath()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Clear any previous debug shapes for this actor
	FlushDebugStrings(World);

	// PERFORMANCE: Use const references and avoid redundant calculations
	const FVector& BasePosition = InitialPosition;
	const FColor DebugColor = FColor::Yellow;
	const float LineThickness = 2.0f;
	const float LifeTime = -1.0f;

	switch (MovementType)
	{
	case EMovementType::Static:
		// No movement, no visualization needed
		break;

	case EMovementType::UpDown:
		DrawVerticalMovementPath(World, BasePosition, DebugColor, LineThickness, LifeTime);
		break;

	case EMovementType::LeftRight:
		DrawHorizontalMovementPath(World, BasePosition, DebugColor, LineThickness, LifeTime);
		break;

	case EMovementType::FrontBack:
		DrawDepthMovementPath(World, BasePosition, DebugColor, LineThickness, LifeTime);
		break;

	case EMovementType::Circular:
		DrawCircularMovementPath(World, BasePosition, DebugColor, LineThickness, LifeTime);
		break;

	case EMovementType::Zigzag:
		DrawZigzagMovementPath(World, BasePosition, DebugColor, LineThickness, LifeTime);
		break;
	}

	// Draw proximity trigger radius if enabled
	if (bProximityTriggered)
	{
		DrawDebugSphere(World, BasePosition, TriggerRadius, 32, FColor::Red, true, LifeTime, 0, 1.0f);
		DrawDebugString(World, BasePosition + FVector(0, 0, TriggerRadius + 20.0f),
			FString::Printf(TEXT("Trigger Radius: %.1f"), TriggerRadius),
			nullptr, FColor::Red, LifeTime, true);
	}
}

void ASpikes::DrawVerticalMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime)
{
	const FVector TopPoint = BasePosition + FVector(0, 0, MaxMovementOffset);
	const FVector BottomPoint = BasePosition - FVector(0, 0, MaxMovementOffset);
	
	DrawDebugLine(World, TopPoint, BottomPoint, Color, true, LifeTime, 0, Thickness);
	DrawDebugString(World, TopPoint, TEXT("Max Height"), nullptr, FColor::White, LifeTime, true);
	DrawDebugString(World, BottomPoint, TEXT("Min Height"), nullptr, FColor::White, LifeTime, true);
}

void ASpikes::DrawHorizontalMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime)
{
	const FVector LeftPoint = BasePosition - FVector(MaxMovementOffset, 0, 0);
	const FVector RightPoint = BasePosition + FVector(MaxMovementOffset, 0, 0);
	
	DrawDebugLine(World, LeftPoint, RightPoint, Color, true, LifeTime, 0, Thickness);
	DrawDebugString(World, LeftPoint, TEXT("Left Extent"), nullptr, FColor::White, LifeTime, true);
	DrawDebugString(World, RightPoint, TEXT("Right Extent"), nullptr, FColor::White, LifeTime, true);
}

void ASpikes::DrawDepthMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime)
{
	const FVector FrontPoint = BasePosition + FVector(0, MaxMovementOffset, 0);
	const FVector BackPoint = BasePosition - FVector(0, MaxMovementOffset, 0);
	
	DrawDebugLine(World, FrontPoint, BackPoint, Color, true, LifeTime, 0, Thickness);
	DrawDebugString(World, FrontPoint, TEXT("Front Extent"), nullptr, FColor::White, LifeTime, true);
	DrawDebugString(World, BackPoint, TEXT("Back Extent"), nullptr, FColor::White, LifeTime, true);
}

void ASpikes::DrawCircularMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime)
{
	constexpr int32 NumSegments = 32;
	constexpr float AngleIncrement = 2.0f * PI / NumSegments;
	
	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float Angle1 = i * AngleIncrement;
		const float Angle2 = (i + 1) * AngleIncrement;

		const FVector Point1 = BasePosition + FVector(
			FMath::Sin(Angle1) * MaxMovementOffset,
			FMath::Cos(Angle1) * MaxMovementOffset,
			0);
		const FVector Point2 = BasePosition + FVector(
			FMath::Sin(Angle2) * MaxMovementOffset,
			FMath::Cos(Angle2) * MaxMovementOffset,
			0);

		DrawDebugLine(World, Point1, Point2, Color, true, LifeTime, 0, Thickness);
	}

	DrawDebugString(World, BasePosition, TEXT("Center"), nullptr, FColor::White, LifeTime, true);
}

void ASpikes::DrawZigzagMovementPath(UWorld* World, const FVector& BasePosition, const FColor& Color, float Thickness, float LifeTime)
{
	constexpr int32 NumPoints = 5;
	const TArray<FVector> Points = {
		BasePosition,
		BasePosition + FVector(MaxMovementOffset * 0.25f, 0, MaxMovementOffset * 0.2f),
		BasePosition + FVector(MaxMovementOffset * 0.5f, 0, 0),
		BasePosition + FVector(MaxMovementOffset * 0.75f, 0, MaxMovementOffset * 0.2f),
		BasePosition + FVector(MaxMovementOffset, 0, 0)
	};

	for (int32 i = 0; i < NumPoints - 1; ++i)
	{
		DrawDebugLine(World, Points[i], Points[i + 1], Color, true, LifeTime, 0, Thickness);
	}

	DrawDebugString(World, Points[0], TEXT("Start"), nullptr, FColor::White, LifeTime, true);
	DrawDebugString(World, Points[NumPoints - 1], TEXT("End"), nullptr, FColor::White, LifeTime, true);
}

void ASpikes::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ?
		PropertyChangedEvent.Property->GetFName() : NAME_None;

	// PERFORMANCE: Only update visualization when relevant properties change
	static const TArray<FName> RelevantProperties = {
		GET_MEMBER_NAME_CHECKED(ASpikes, MovementType),
		GET_MEMBER_NAME_CHECKED(ASpikes, MaxMovementOffset),
		GET_MEMBER_NAME_CHECKED(ASpikes, bProximityTriggered),
		GET_MEMBER_NAME_CHECKED(ASpikes, TriggerRadius)
	};

	if (RelevantProperties.Contains(PropertyName))
	{
		DrawDebugMovementPath();
	}
}
#endif