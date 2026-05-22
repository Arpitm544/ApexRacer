#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "RacingGameState.generated.h"

class ACheckpointActor;

UENUM(BlueprintType)
enum class ERaceState : uint8
{
	PreRaceCountdown UMETA(DisplayName = "Pre-Race Countdown"),
	Racing           UMETA(DisplayName = "Racing"),
	Finished         UMETA(DisplayName = "Finished")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRaceStateChangedSignature, ERaceState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownTickSignature, int32, SecondsRemaining);

/**
 * GameState managing the current race session, checkpoint catalog, countdown timer, and global standings.
 */
UCLASS()
class APEXRACER_API ARacingGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ARacingGameState();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Racing|State")
	void SetRaceState(ERaceState NewState);

	UFUNCTION(BlueprintPure, Category = "Racing|State")
	ERaceState GetRaceState() const { return CurrentRaceState; }

	UFUNCTION(BlueprintPure, Category = "Racing|State")
	float GetCountdownRemaining() const { return CountdownRemaining; }

	UFUNCTION(BlueprintPure, Category = "Racing|State")
	int32 GetTrackCheckpointCount() const { return TrackCheckpoints.Num(); }

	UFUNCTION(BlueprintPure, Category = "Racing|State")
	ACheckpointActor* GetCheckpointByIndex(int32 Index) const;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnRaceStateChangedSignature OnRaceStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnCountdownTickSignature OnCountdownTick;

	void DiscoverAndRegisterCheckpoints();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|State")
	ERaceState CurrentRaceState = ERaceState::PreRaceCountdown;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Countdown")
	float CountdownDuration = 3.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Countdown")
	float CountdownRemaining = 3.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Track")
	TArray<TObjectPtr<ACheckpointActor>> TrackCheckpoints;

private:
	int32 LastDispatchedCountdownSecond = -1;
};
