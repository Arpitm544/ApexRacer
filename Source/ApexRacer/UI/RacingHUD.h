#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RacingHUD.generated.h"

class ARacingVehiclePawn;
class ARacingPlayerController;
class ULapTimingComponent;
class ARacingGameState;

/**
 * Racing HUD supporting both built-in C++ Canvas drawing (immediate out-of-the-box UI)
 * and optional UMG Widget overlays.
 */
UCLASS()
class APEXRACER_API ARacingHUD : public AHUD
{
	GENERATED_BODY()

public:
	ARacingHUD();

	virtual void DrawHUD() override;

protected:
	void DrawSpeedometer(float ScreenX, float ScreenY, ARacingVehiclePawn* Vehicle);
	void DrawTachometer(float ScreenX, float ScreenY, ARacingVehiclePawn* Vehicle);
	void DrawTimingBoard(float ScreenX, float ScreenY, ULapTimingComponent* LapTimer);
	void DrawCountdownOverlay(ARacingGameState* GameState);
	void DrawWrongWayWarning();
	void DrawFinishedBanner(ULapTimingComponent* LapTimer);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|UI")
	bool bEnableCanvasHUD = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|UI")
	TSubclassOf<class UUserWidget> UMGHUDClass;

	UPROPERTY(Transient)
	TObjectPtr<class UUserWidget> ActiveUMGHUDWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|UI")
	TObjectPtr<UFont> HUDFont;
};
