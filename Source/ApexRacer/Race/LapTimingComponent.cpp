#include "Race/LapTimingComponent.h"
#include "ApexRacer.h"
#include "Track/CheckpointActor.h"

ULapTimingComponent::ULapTimingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.0f; // High-precision frame tick
}

void ULapTimingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULapTimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bRaceStarted && !bRaceFinished)
	{
		CurrentLapTime += DeltaTime;
	}
}

void ULapTimingComponent::StartTiming(int32 InTotalLaps, int32 TotalTrackCheckpoints)
{
	TotalLaps = FMath::Max(1, InTotalLaps);
	TotalCheckpointsCount = FMath::Max(1, TotalTrackCheckpoints);

	CurrentLap = 1;
	CurrentLapTime = 0.0f;
	BestLapTime = 0.0f;
	LastLapTime = 0.0f;
	NextExpectedCheckpointIndex = 1; // Expect checkpoint 1 first (0 is start/finish)
	bRaceStarted = true;
	bRaceFinished = false;
	bWrongWay = false;

	LastSectorTimestamp = 0.0f;
	CurrentSectorTimes.Init(0.0f, TotalCheckpointsCount);
	BestSectorTimes.Init(0.0f, TotalCheckpointsCount);

	UE_LOG(LogApexRacer, Log, TEXT("Lap timing started: %d laps, %d track checkpoints."), TotalLaps, TotalCheckpointsCount);
}

void ULapTimingComponent::FinishTiming()
{
	bRaceFinished = true;
	UE_LOG(LogApexRacer, Log, TEXT("Race Finished! Final Lap: %d/%d, Best Lap: %s"),
	       CurrentLap, TotalLaps, *FormatLapTime(BestLapTime));
}

void ULapTimingComponent::RegisterCheckpointPassage(ACheckpointActor* Checkpoint, bool bValidDirection)
{
	if (!Checkpoint || !bRaceStarted || bRaceFinished)
	{
		return;
	}

	int32 PassedIndex = Checkpoint->CheckpointIndex;

	// Check for wrong-way driving
	if (!bValidDirection)
	{
		if (!bWrongWay)
		{
			bWrongWay = true;
			OnWrongWayStateChanged.Broadcast(true);
			UE_LOG(LogApexRacer, Warning, TEXT("WRONG WAY! Vehicle entered checkpoint %d against flow."), PassedIndex);
		}
		return;
	}
	else if (bWrongWay)
	{
		bWrongWay = false;
		OnWrongWayStateChanged.Broadcast(false);
	}

	// 1. Finish Line / Lap Completion Logic
	if (Checkpoint->bIsFinishLine)
	{
		// Finish line only counts if all intermediate checkpoints were passed (NextExpectedCheckpointIndex == 0)
		if (NextExpectedCheckpointIndex == 0)
		{
			LastLapTime = CurrentLapTime;
			bool bIsNewBest = (BestLapTime <= 0.0f || CurrentLapTime < BestLapTime);

			if (bIsNewBest)
			{
				BestLapTime = CurrentLapTime;
				BestSectorTimes = CurrentSectorTimes;
				OnNewBestLap.Broadcast(CurrentLap, BestLapTime);
			}

			OnLapCompleted.Broadcast(CurrentLap);
			UE_LOG(LogApexRacer, Log, TEXT("Lap %d completed in: %s (Best: %s)"),
			       CurrentLap, *FormatLapTime(LastLapTime), *FormatLapTime(BestLapTime));

			if (CurrentLap >= TotalLaps)
			{
				FinishTiming();
				return;
			}

			// Advance to next lap
			CurrentLap++;
			CurrentLapTime = 0.0f;
			LastSectorTimestamp = 0.0f;
			NextExpectedCheckpointIndex = 1;
		}
		return;
	}

	// 2. Intermediate Checkpoint Passage
	if (PassedIndex == NextExpectedCheckpointIndex)
	{
		float SectorTime = CurrentLapTime - LastSectorTimestamp;
		LastSectorTimestamp = CurrentLapTime;

		if (CurrentSectorTimes.IsValidIndex(PassedIndex))
		{
			CurrentSectorTimes[PassedIndex] = SectorTime;
		}

		float SplitDelta = 0.0f;
		if (BestSectorTimes.IsValidIndex(PassedIndex) && BestSectorTimes[PassedIndex] > 0.0f)
		{
			SplitDelta = SectorTime - BestSectorTimes[PassedIndex]; // Negative = faster (green), Positive = slower (red)
		}

		OnSectorSplit.Broadcast(PassedIndex, SectorTime, SplitDelta);

		// Advance expected gate
		NextExpectedCheckpointIndex++;
		if (NextExpectedCheckpointIndex >= TotalCheckpointsCount)
		{
			// All intermediate gates cleared, next valid gate is the finish line (index 0)
			NextExpectedCheckpointIndex = 0;
		}

		UE_LOG(LogApexRacer, Verbose, TEXT("Checkpoint %d cleared. Next required: %d"),
		       PassedIndex, NextExpectedCheckpointIndex);
	}
}

FString ULapTimingComponent::FormatLapTime(float TimeInSeconds)
{
	if (TimeInSeconds <= 0.0f)
	{
		return TEXT("--:--.---");
	}

	int32 Minutes = FMath::FloorToInt(TimeInSeconds / 60.0f);
	float RemainingSeconds = TimeInSeconds - (Minutes * 60.0f);
	int32 Seconds = FMath::FloorToInt(RemainingSeconds);
	int32 Milliseconds = FMath::FloorToInt((RemainingSeconds - Seconds) * 1000.0f);

	return FString::Printf(TEXT("%02d:%02d.%03d"), Minutes, Seconds, Milliseconds);
}
