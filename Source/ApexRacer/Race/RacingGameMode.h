#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RacingGameMode.generated.h"

/**
 * GameMode governing rules, start sequence, and player spawning for the 3D racing game.
 */
UCLASS()
class APEXRACER_API ARacingGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARacingGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	// Restart the race session
	UFUNCTION(BlueprintCallable, Category = "Racing|GameMode")
	void RestartRace();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Racing|Rules")
	int32 TotalRaceLaps = 3;
};
