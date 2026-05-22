#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Race/RacingGameState.h"
#include "RacingPlayerController.generated.h"

class ARacingVehiclePawn;
class ACheckpointActor;
class ULapTimingComponent;

/**
 * Custom PlayerController for 3D Racing.
 * Coordinates input states with race countdown and handles vehicle recovery.
 */
UCLASS()
class APEXRACER_API ARacingPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARacingPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	// Recovers vehicle to the last valid checkpoint location
	UFUNCTION(BlueprintCallable, Category = "Racing|Recovery")
	void RecoverVehicleToLastCheckpoint();

	// Sets the last valid checkpoint for vehicle reset
	void SetLastValidCheckpoint(ACheckpointActor* Checkpoint);

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	ULapTimingComponent* GetLapTimingComponent() const { return LapTimer; }

protected:
	UFUNCTION()
	void HandleRaceStateChanged(ERaceState NewState);

	UFUNCTION()
	void HandleCheckpointTriggered(ACheckpointActor* Checkpoint, AActor* OverlappedActor, bool bValidDirection);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<ULapTimingComponent> LapTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Recovery")
	TObjectPtr<ACheckpointActor> LastValidCheckpoint;
};
