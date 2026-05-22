#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "InputActionValue.h"
#include "RacingVehiclePawn.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UChaosWheeledVehicleMovementComponent;
class UInputMappingContext;
class UInputAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearChangedSignature, int32, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTireSkidSignature, int32, WheelIndex, float, SkidIntensity);

/**
 * High-performance 3D Racing Vehicle Pawn powered by UE Chaos Physics.
 * Provides camera handling, dynamic FOV, enhanced input, telemetry, and vehicle reset.
 */
UCLASS()
class APEXRACER_API ARacingVehiclePawn : public AWheeledVehiclePawn
{
	GENERATED_BODY()

public:
	ARacingVehiclePawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Telemetry Getters
	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	float GetVehicleSpeedKMH() const;

	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	float GetVehicleSpeedMPH() const;

	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	float GetEngineRPM() const;

	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	int32 GetCurrentGear() const;

	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	float GetDriftAngle() const;

	UFUNCTION(BlueprintPure, Category = "Racing|Telemetry")
	bool IsTireSlipping() const;

	// Reset vehicle to a safe upright position & zero velocities
	UFUNCTION(BlueprintCallable, Category = "Racing|Vehicle")
	void ResetVehicle(const FVector& NewLocation, const FRotator& NewRotation);

	// Input Actions
	UFUNCTION(BlueprintCallable, Category = "Racing|Input")
	void ApplyThrottle(float Value);

	UFUNCTION(BlueprintCallable, Category = "Racing|Input")
	void ApplySteering(float Value);

	UFUNCTION(BlueprintCallable, Category = "Racing|Input")
	void ApplyBrake(float Value);

	UFUNCTION(BlueprintCallable, Category = "Racing|Input")
	void SetHandbrake(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Racing|Input")
	void ToggleCameraView();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnGearChangedSignature OnGearChanged;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnTireSkidSignature OnTireSkid;

protected:
	// Enhanced Input bindings
	void InputThrottle(const FInputActionValue& Value);
	void InputSteering(const FInputActionValue& Value);
	void InputBrake(const FInputActionValue& Value);
	void InputHandbrakeStarted(const FInputActionValue& Value);
	void InputHandbrakeCompleted(const FInputActionValue& Value);
	void InputToggleCamera(const FInputActionValue& Value);
	void InputResetVehicle(const FInputActionValue& Value);

	// Fallback Legacy Axis bindings
	void AxisThrottle(float Value);
	void AxisSteering(float Value);
	void AxisBrake(float Value);
	void ActionHandbrakePressed();
	void ActionHandbrakeReleased();

	void UpdateCameraDynamics(float DeltaSeconds);
	void CheckTireSlip(float DeltaSeconds);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<USpringArmComponent> ChaseSpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UCameraComponent> ChaseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UCameraComponent> HoodCamera;

	// Enhanced Input Assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputMappingContext> RacingInputContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionThrottle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionSteering;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionBrake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionHandbrake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionToggleCam;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Input")
	TObjectPtr<UInputAction> ActionReset;

	// Dynamic Camera Tuning
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Camera")
	float BaseFOV = 88.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Camera")
	float MaxFOV = 108.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Camera")
	float FOVSpeedThresholdKMH = 200.0f;

	// Skid Sensitivity
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Physics")
	float SlipThreshold = 0.28f;

private:
	int32 LastReportedGear = 0;
	bool bIsUsingHoodCamera = false;
	FTransform LastSafeCheckpointTransform;
};
