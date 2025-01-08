#include "RunnerCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MemoryEchoSystem.h"
// #include "NPCInteractionManager.h"  // Commented out
#include "Kismet/GameplayStatics.h"

ARunnerCharacter::ARunnerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Set size for collision capsule
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

    // Configure character movement
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, RotationRate, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 600.f;
    GetCharacterMovement()->AirControl = 0.2f;

    // Create camera
    SideViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("SideViewCamera"));
    SideViewCamera->SetupAttachment(RootComponent);
    SideViewCamera->bUsePawnControlRotation = false;

    // Create core systems
    MemorySystem = CreateDefaultSubobject<UMemoryEchoSystem>(TEXT("MemorySystem"));
    // NPCManager = CreateDefaultSubobject<UNPCInteractionManager>(TEXT("NPCManager"));  // Commented out

    // Set default values
    bCanDoubleJump = true;
    DoubleJumpZVelocity = 600.0f;
    CanMove = true;
    CanJump = true;
    CanDoubleJump = true;
    zPosition = 0.0f;
}

void ARunnerCharacter::BeginPlay()
{
    Super::BeginPlay();

    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ARunnerCharacter::OnOverlapBegin);
    }

    if (GetCharacterMovement())
    {
        JumpZVelocity = GetCharacterMovement()->JumpZVelocity;
    }
}

void ARunnerCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TempPos = GetActorLocation();
    TempPos.Z = zPosition;
    SetActorLocation(TempPos);
}

void ARunnerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARunnerCharacter::Jump);
    PlayerInputComponent->BindAxis("MoveRight", this, &ARunnerCharacter::MoveRight);
}

void ARunnerCharacter::Jump()
{
    if (CanJump)
    {
        if (JumpCurrentCount == 0)
        {
            Super::Jump();
        }
        else if (JumpCurrentCount == 1 && bCanDoubleJump)
        {
            GetCharacterMovement()->Velocity.Z = DoubleJumpZVelocity;
            JumpCurrentCount++;
        }
    }
}

void ARunnerCharacter::MoveRight(float Value)
{
    if (CanMove && (Controller != nullptr))
    {
        AddMovementInput(FVector(1.0f, 0.0f, 0.0f), Value);
    }
}

void ARunnerCharacter::RestartLevel()
{
    UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void ARunnerCharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor != nullptr)
    {
        DeathOfPlayer();
    }
}

void ARunnerCharacter::RecordMemory(const FString& Content)
{
    if (MemorySystem)
    {
        MemorySystem->RecordMemory(Content);
    }
}

// Commented out InteractWithNPC function entirely
/*
void ARunnerCharacter::InteractWithNPC(const FString& NPCName, const FString& DialogueText)
{
    if (NPCManager)
    {
        if (!NPCManager->IsValidNPC(NPCName))
        {
            NPCManager->InitializeNPC(NPCName);
        }
        NPCManager->AddDialogueEn
*/