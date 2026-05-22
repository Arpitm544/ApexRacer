#include "Vehicle/RacingVehiclePawn.h"
#include "ApexRacer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/SkeletalMeshComponent.h"

ARacingVehiclePawn::ARacingVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Spring Arm for Third Person Chase View
	ChaseSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("ChaseSpringArm"));
	ChaseSpringArm->SetupAttachment(RootComponent);
	ChaseSpringArm->TargetArmLength = 550.0f;
	ChaseSpringArm->SocketOffset = FVector(0.0f, 0.0f, 160.0f);
	ChaseSpringArm->bEnableCameraLag = true;
	ChaseSpringArm->CameraLagSpeed = 16.0f;
	ChaseSpringArm->bEnableCameraRotationLag = true;
	ChaseSpringArm->CameraRotationLagSpeed = 18.0f;
	ChaseSpringArm->bInheritPitch = true;
	ChaseSpringArm->bInheritYaw = true;
	ChaseSpringArm->bInheritRoll = false; // Keep camera horizon stable during roll

	// Chase Camera
	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(ChaseSpringArm, USpringArmComponent::SocketName);
	ChaseCamera->FieldOfView = BaseFOV;
	ChaseCamera->bAutoActivate = true;

	// Hood / Bumper Camera for immersive first-person racing
	HoodCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("HoodCamera"));
	HoodCamera->SetupAttachment(RootComponent);
	HoodCamera->SetRelativeLocation(FVector(60.0f, 0.0f, 95.0f));
	HoodCamera->bAutoActivate = false;
}

void ARacingVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Register Enhanced Input Mapping Context
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (RacingInputContext)
			{
				Subsystem->AddMappingContext(RacingInputContext, 0);
			}
		}
	}

	LastSafeCheckpointTransform = GetActorTransform();
}

void ARacingVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraDynamics(DeltaSeconds);
	CheckTireSlip(DeltaSeconds);

	// Gear change notification
	int32 CurrentGear = GetCurrentGear();
	if (CurrentGear != LastReportedGear)
	{
		LastReportedGear = CurrentGear;
		OnGearChanged.Broadcast(CurrentGear);
	}
}

void ARacingVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (ActionThrottle)
		{
			EnhancedInputComponent->BindAction(ActionThrottle, ETriggerEvent::Triggered, this, &ARacingVehiclePawn::InputThrottle);
			EnhancedInputComponent->BindAction(ActionThrottle, ETriggerEvent::Completed, this, &ARacingVehiclePawn::InputThrottle);
		}

		if (ActionSteering)
		{
			EnhancedInputComponent->BindAction(ActionSteering, ETriggerEvent::Triggered, this, &ARacingVehiclePawn::InputSteering);
			EnhancedInputComponent->BindAction(ActionSteering, ETriggerEvent::Completed, this, &ARacingVehiclePawn::InputSteering);
		}

		if (ActionBrake)
		{
			EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Triggered, this, &ARacingVehiclePawn::InputBrake);
			EnhancedInputComponent->BindAction(ActionBrake, ETriggerEvent::Completed, this, &ARacingVehiclePawn::InputBrake);
		}

		if (ActionHandbrake)
		{
			EnhancedInputComponent->BindAction(ActionHandbrake, ETriggerEvent::Started, this, &ARacingVehiclePawn::InputHandbrakeStarted);
			EnhancedInputComponent->BindAction(ActionHandbrake, ETriggerEvent::Completed, this, &ARacingVehiclePawn::InputHandbrakeCompleted);
		}

		if (ActionToggleCam)
		{
			EnhancedInputComponent->BindAction(ActionToggleCam, ETriggerEvent::Started, this, &ARacingVehiclePawn::InputToggleCamera);
		}

		if (ActionReset)
		{
			EnhancedInputComponent->BindAction(ActionReset, ETriggerEvent::Started, this, &ARacingVehiclePawn::InputResetVehicle);
		}
	}

	// Legacy Axis/Action Bindings as fallback
	PlayerInputComponent->BindAxis("Throttle", this, &ARacingVehiclePawn::AxisThrottle);
	PlayerInputComponent->BindAxis("Steering", this, &ARacingVehiclePawn::AxisSteering);
	PlayerInputComponent->BindAxis("BrakeReverse", this, &ARacingVehiclePawn::AxisBrake);
	PlayerInputComponent->BindAction("Handbrake", IE_Pressed, this, &ARacingVehiclePawn::ActionHandbrakePressed);
	PlayerInputComponent->BindAction("Handbrake", IE_Released, this, &ARacingVehiclePawn::ActionHandbrakeReleased);
	PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &ARacingVehiclePawn::ToggleCameraView);
	PlayerInputComponent->BindAction("ResetCar", IE_Pressed, this, &ARacingVehiclePawn::InputResetVehicle, FInputActionValue());
}

// Telemetry & Physics Queries
float ARacingVehiclePawn::GetVehicleSpeedKMH() const
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		return FMath::Abs(Movement->GetForwardSpeedMPH() * 1.60934f);
	}
	return 0.0f;
}

float ARacingVehiclePawn::GetVehicleSpeedMPH() const
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		return FMath::Abs(Movement->GetForwardSpeedMPH());
	}
	return 0.0f;
}

float ARacingVehiclePawn::GetEngineRPM() const
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		return Movement->GetEngineRotationSpeed();
	}
	return 0.0f;
}

int32 ARacingVehiclePawn::GetCurrentGear() const
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		return Movement->GetCurrentGear();
	}
	return 0;
}

float ARacingVehiclePawn::GetDriftAngle() const
{
	FVector Forward = GetActorForwardVector();
	FVector Velocity = GetVelocity();

	if (Velocity.SizeSquared2D() < 10000.0f) // Below ~10 km/h
	{
		return 0.0f;
	}

	Velocity.Normalize();
	float Dot = FVector::DotProduct(Forward, Velocity);
	float AngleRad = FMath::Acos(FMath::Clamp(Dot, -1.0f, 1.0f));
	return FMath::RadiansToDegrees(AngleRad);
}

bool ARacingVehiclePawn::IsTireSlipping() const
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		int32 NumWheels = Movement->Wheels.Num();
		for (int32 i = 0; i < NumWheels; ++i)
		{
			if (FMath::Abs(Movement->GetWheelSlipRatio(i)) > SlipThreshold ||
			    FMath::Abs(Movement->GetWheelSkidMagnitude(i)) > SlipThreshold)
			{
				return true;
			}
		}
	}
	return false;
}

void ARacingVehiclePawn::ResetVehicle(const FVector& NewLocation, const FRotator& NewRotation)
{
	FRotator LevelRotation = NewRotation;
	LevelRotation.Pitch = 0.0f;
	LevelRotation.Roll = 0.0f;

	// Lift slightly above ground to prevent clipping
	FVector SafeLocation = NewLocation + FVector(0.0f, 0.0f, 35.0f);

	SetActorLocationAndRotation(SafeLocation, LevelRotation, false, nullptr, ETeleportType::TeleportPhysics);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetPhysicsLinearVelocity(FVector::ZeroVector);
		MeshComp->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Movement->SetThrottleInput(0.0f);
		Movement->SetBrakeInput(0.0f);
		Movement->SetSteeringInput(0.0f);
		Movement->SetHandbrakeInput(false);
	}

	UE_LOG(LogApexRacer, Log, TEXT("Vehicle reset upright at: %s"), *SafeLocation.ToString());
}

// Input Handlers
void ARacingVehiclePawn::ApplyThrottle(float Value)
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Movement->SetThrottleInput(Value);
	}
}

void ARacingVehiclePawn::ApplySteering(float Value)
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Movement->SetSteeringInput(Value);
	}
}

void ARacingVehiclePawn::ApplyBrake(float Value)
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Movement->SetBrakeInput(Value);
	}
}

void ARacingVehiclePawn::SetHandbrake(bool bActive)
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		Movement->SetHandbrakeInput(bActive);
	}
}

void ARacingVehiclePawn::ToggleCameraView()
{
	bIsUsingHoodCamera = !bIsUsingHoodCamera;

	if (bIsUsingHoodCamera)
	{
		ChaseCamera->SetActive(false);
		HoodCamera->SetActive(true);
	}
	else
	{
		HoodCamera->SetActive(false);
		ChaseCamera->SetActive(true);
	}
}

// Enhanced Input callbacks
void ARacingVehiclePawn::InputThrottle(const FInputActionValue& Value)
{
	ApplyThrottle(Value.Get<float>());
}

void ARacingVehiclePawn::InputSteering(const FInputActionValue& Value)
{
	ApplySteering(Value.Get<float>());
}

void ARacingVehiclePawn::InputBrake(const FInputActionValue& Value)
{
	ApplyBrake(Value.Get<float>());
}

void ARacingVehiclePawn::InputHandbrakeStarted(const FInputActionValue& Value)
{
	SetHandbrake(true);
}

void ARacingVehiclePawn::InputHandbrakeCompleted(const FInputActionValue& Value)
{
	SetHandbrake(false);
}

void ARacingVehiclePawn::InputToggleCamera(const FInputActionValue& Value)
{
	ToggleCameraView();
}

void ARacingVehiclePawn::InputResetVehicle(const FInputActionValue& Value)
{
	// Reset to actor current XY but oriented upright
	ResetVehicle(GetActorLocation(), GetActorRotation());
}

// Legacy Axis / Action callbacks
void ARacingVehiclePawn::AxisThrottle(float Value) { ApplyThrottle(Value); }
void ARacingVehiclePawn::AxisSteering(float Value) { ApplySteering(Value); }
void ARacingVehiclePawn::AxisBrake(float Value) { ApplyBrake(Value); }
void ARacingVehiclePawn::ActionHandbrakePressed() { SetHandbrake(true); }
void ARacingVehiclePawn::ActionHandbrakeReleased() { SetHandbrake(false); }

void ARacingVehiclePawn::UpdateCameraDynamics(float DeltaSeconds)
{
	if (!ChaseCamera || !ChaseCamera->IsActive())
	{
		return;
	}

	float CurrentSpeed = GetVehicleSpeedKMH();
	float SpeedAlpha = FMath::Clamp(CurrentSpeed / FOVSpeedThresholdKMH, 0.0f, 1.0f);
	float TargetFOV = FMath::Lerp(BaseFOV, MaxFOV, SpeedAlpha);

	ChaseCamera->FieldOfView = FMath::FInterpTo(ChaseCamera->FieldOfView, TargetFOV, DeltaSeconds, 4.0f);
}

void ARacingVehiclePawn::CheckTireSlip(float DeltaSeconds)
{
	if (UChaosWheeledVehicleMovementComponent* Movement = Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement()))
	{
		int32 NumWheels = Movement->Wheels.Num();
		for (int32 i = 0; i < NumWheels; ++i)
		{
			float SkidMag = FMath::Abs(Movement->GetWheelSkidMagnitude(i));
			if (SkidMag > SlipThreshold)
			{
				OnTireSkid.Broadcast(i, SkidMag);
			}
		}
	}
}
