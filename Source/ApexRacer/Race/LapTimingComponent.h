#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LapTimingComponent.generated.h"

class ACheckpointActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLapCompletedSignature, int32, CompletedLap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNewBestLapSignature, int32, LapNumber, float, BestTimeSeconds);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSectorSplitSignature, int32, SectorIndex, float, SectorTime, float, SplitDelta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWrongWayStateChangedSignature, bool, bIsWrongWay);

/**
 * High-precision lap timer and anti-cheat checkpoint tracking component.
 * Validates sequential gate passage, detects wrong-way driving, and records sector splits.
 */
UCLASS(ClassGroup=(Racing), meta=(BlueprintSpawnableComponent))
class APEXRACER_API ULapTimingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULapTimingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Reset race timing for a fresh start
	UFUNCTION(BlueprintCallable, Category = "Racing|Timing")
	void StartTiming(int32 InTotalLaps, int32 TotalTrackCheckpoints);

	// Stop timing when race finishes
	UFUNCTION(BlueprintCallable, Category = "Racing|Timing")
	void FinishTiming();

	// Processes checkpoint crossing event from a CheckpointActor
	UFUNCTION(BlueprintCallable, Category = "Racing|Timing")
	void RegisterCheckpointPassage(ACheckpointActor* Checkpoint, bool bValidDirection);

	// Telemetry & Timing Getters
	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	float GetCurrentLapTime() const { return CurrentLapTime; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	float GetBestLapTime() const { return BestLapTime; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	float GetLastLapTime() const { return LastLapTime; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	int32 GetCurrentLap() const { return CurrentLap; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	int32 GetTotalLaps() const { return TotalLaps; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	bool IsRaceCompleted() const { return bRaceFinished; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	bool IsWrongWay() const { return bWrongWay; }

	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	int32 GetNextExpectedCheckpointIndex() const { return NextExpectedCheckpointIndex; }

	// Utility to format seconds into "MM:SS.mmm"
	UFUNCTION(BlueprintPure, Category = "Racing|Timing")
	static FString FormatLapTime(float TimeInSeconds);

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnLapCompletedSignature OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnNewBestLapSignature OnNewBestLap;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnSectorSplitSignature OnSectorSplit;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnWrongWayStateChangedSignature OnWrongWayStateChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	int32 CurrentLap = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Timing")
	int32 TotalLaps = 3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	float CurrentLapTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	float BestLapTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	float LastLapTime = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	int32 NextExpectedCheckpointIndex = 1; // 0 is Start/Finish line

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	int32 TotalCheckpointsCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	bool bRaceStarted = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	bool bRaceFinished = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	bool bWrongWay = false;

	// Sector split tracking
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	TArray<float> CurrentSectorTimes;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Timing")
	TArray<float> BestSectorTimes;

	float LastSectorTimestamp = 0.0f;
};
