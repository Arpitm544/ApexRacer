#include "Race/RacingGameState.h"
#include "ApexRacer.h"
#include "Track/CheckpointActor.h"
#include "Kismet/GameplayStatics.h"

ARacingGameState::ARacingGameState()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARacingGameState::BeginPlay()
{
	Super::BeginPlay();

	CountdownRemaining = CountdownDuration;
	DiscoverAndRegisterCheckpoints();
}

void ARacingGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentRaceState == ERaceState::PreRaceCountdown)
	{
		CountdownRemaining -= DeltaSeconds;

		int32 DisplaySecond = FMath::CeilToInt(CountdownRemaining);
		if (DisplaySecond != LastDispatchedCountdownSecond && DisplaySecond >= 0)
		{
			LastDispatchedCountdownSecond = DisplaySecond;
			OnCountdownTick.Broadcast(DisplaySecond);
		}

		if (CountdownRemaining <= 0.0f)
		{
			SetRaceState(ERaceState::Racing);
		}
	}
}

void ARacingGameState::SetRaceState(ERaceState NewState)
{
	if (CurrentRaceState != NewState)
	{
		CurrentRaceState = NewState;
		UE_LOG(LogApexRacer, Log, TEXT("Race State changed to: %d"), static_cast<int32>(NewState));
		OnRaceStateChanged.Broadcast(CurrentRaceState);
	}
}

void ARacingGameState::DiscoverAndRegisterCheckpoints()
{
	TrackCheckpoints.Empty();

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpointActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (ACheckpointActor* CP = Cast<ACheckpointActor>(Actor))
		{
			TrackCheckpoints.Add(CP);
		}
	}

	// Sort checkpoints by index
	TrackCheckpoints.Sort([](const TObjectPtr<ACheckpointActor>& A, const TObjectPtr<ACheckpointActor>& B) {
		return A->CheckpointIndex < B->CheckpointIndex;
	});

	UE_LOG(LogApexRacer, Log, TEXT("Discovered %d track checkpoints in level."), TrackCheckpoints.Num());
}

ACheckpointActor* ARacingGameState::GetCheckpointByIndex(int32 Index) const
{
	for (const TObjectPtr<ACheckpointActor>& CP : TrackCheckpoints)
	{
		if (CP && CP->CheckpointIndex == Index)
		{
			return CP;
		}
	}
	return nullptr;
}
