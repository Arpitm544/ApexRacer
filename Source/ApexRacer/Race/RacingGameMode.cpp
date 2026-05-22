#include "Race/RacingGameMode.h"
#include "ApexRacer.h"
#include "Vehicle/RacingVehiclePawn.h"
#include "Race/RacingGameState.h"
#include "Race/RacingPlayerController.h"
#include "UI/RacingHUD.h"
#include "Kismet/GameplayStatics.h"

ARacingGameMode::ARacingGameMode()
{
	DefaultPawnClass = ARacingVehiclePawn::StaticClass();
	GameStateClass = ARacingGameState::StaticClass();
	PlayerControllerClass = ARacingPlayerController::StaticClass();
	HUDClass = ARacingHUD::StaticClass();
}

void ARacingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	UE_LOG(LogApexRacer, Log, TEXT("Player logged in to RacingGameMode: %s"), *NewPlayer->GetName());
}

void ARacingGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void ARacingGameMode::RestartRace()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}
