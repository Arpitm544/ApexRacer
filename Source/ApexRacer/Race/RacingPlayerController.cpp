#include "Race/RacingPlayerController.h"
#include "ApexRacer.h"
#include "Vehicle/RacingVehiclePawn.h"
#include "Track/CheckpointActor.h"
#include "Race/LapTimingComponent.h"
#include "Race/RacingGameState.h"
#include "Kismet/GameplayStatics.h"

ARacingPlayerController::ARacingPlayerController()
{
	LapTimer = CreateDefaultSubobject<ULapTimingComponent>(TEXT("LapTimer"));
}

void ARacingPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ARacingGameState* GS = GetWorld()->GetGameState<ARacingGameState>())
	{
		GS->OnRaceStateChanged.AddDynamic(this, &ARacingPlayerController::HandleRaceStateChanged);

		// If still in countdown, freeze vehicle controls
		if (GS->GetRaceState() == ERaceState::PreRaceCountdown)
		{
			SetIgnoreMoveInput(true);
		}
	}

	// Listen to all checkpoints in level
	TArray<AActor*> FoundCheckpoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpointActor::StaticClass(), FoundCheckpoints);
	for (AActor* Actor : FoundCheckpoints)
	{
		if (ACheckpointActor* CP = Cast<ACheckpointActor>(Actor))
		{
			CP->OnCheckpointTriggered.AddDynamic(this, &ARacingPlayerController::HandleCheckpointTriggered);
		}
	}
}

void ARacingPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ARacingGameState* GS = GetWorld()->GetGameState<ARacingGameState>())
	{
		LapTimer->StartTiming(3, GS->GetTrackCheckpointCount());
	}
	else
	{
		LapTimer->StartTiming(3, 4);
	}
}

void ARacingPlayerController::HandleRaceStateChanged(ERaceState NewState)
{
	if (NewState == ERaceState::Racing)
	{
		SetIgnoreMoveInput(false);
		UE_LOG(LogApexRacer, Log, TEXT("Race Started! Input unlocked for player."));
	}
	else if (NewState == ERaceState::Finished)
	{
		LapTimer->FinishTiming();
		UE_LOG(LogApexRacer, Log, TEXT("Player completed the race."));
	}
}

void ARacingPlayerController::HandleCheckpointTriggered(ACheckpointActor* Checkpoint, AActor* OverlappedActor, bool bValidDirection)
{
	if (OverlappedActor == GetPawn())
	{
		LapTimer->RegisterCheckpointPassage(Checkpoint, bValidDirection);

		if (bValidDirection)
		{
			SetLastValidCheckpoint(Checkpoint);
		}
	}
}

void ARacingPlayerController::SetLastValidCheckpoint(ACheckpointActor* Checkpoint)
{
	LastValidCheckpoint = Checkpoint;
}

void ARacingPlayerController::RecoverVehicleToLastCheckpoint()
{
	ARacingVehiclePawn* Vehicle = Cast<ARacingVehiclePawn>(GetPawn());
	if (!Vehicle)
	{
		return;
	}

	if (LastValidCheckpoint)
	{
		FTransform SpawnTransform = LastValidCheckpoint->GetSpawnTransform();
		Vehicle->ResetVehicle(SpawnTransform.GetLocation(), SpawnTransform.GetRotation().Rotator());
	}
	else
	{
		// Default recovery: reset upright at current position
		Vehicle->ResetVehicle(Vehicle->GetActorLocation(), Vehicle->GetActorRotation());
	}
}
