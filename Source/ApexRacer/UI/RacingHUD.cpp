#include "UI/RacingHUD.h"
#include "ApexRacer.h"
#include "Engine/Canvas.h"
#include "Vehicle/RacingVehiclePawn.h"
#include "Race/RacingPlayerController.h"
#include "Race/LapTimingComponent.h"
#include "Race/RacingGameState.h"
#include "Blueprint/UserWidget.h"

ARacingHUD::ARacingHUD()
{
	static ConstructorHelpers::FObjectFinder<UFont> FontFinder(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	if (FontFinder.Succeeded())
	{
		HUDFont = FontFinder.Object;
	}
}

void ARacingHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!bEnableCanvasHUD || !Canvas)
	{
		return;
	}

	ARacingPlayerController* PC = Cast<ARacingPlayerController>(GetOwningPlayerController());
	if (!PC)
	{
		return;
	}

	ARacingVehiclePawn* Vehicle = Cast<ARacingVehiclePawn>(PC->GetPawn());
	ULapTimingComponent* LapTimer = PC->GetLapTimingComponent();
	ARacingGameState* GameState = GetWorld()->GetGameState<ARacingGameState>();

	float ViewportW = Canvas->ClipX;
	float ViewportH = Canvas->ClipY;

	// 1. Draw Speedometer & Tachometer at bottom-right
	if (Vehicle)
	{
		DrawSpeedometer(ViewportW - 280.0f, ViewportH - 180.0f, Vehicle);
		DrawTachometer(ViewportW - 280.0f, ViewportH - 60.0f, Vehicle);
	}

	// 2. Draw Timing & Lap Board at top-left
	if (LapTimer)
	{
		DrawTimingBoard(40.0f, 40.0f, LapTimer);

		if (LapTimer->IsWrongWay())
		{
			DrawWrongWayWarning();
		}

		if (LapTimer->IsRaceCompleted())
		{
			DrawFinishedBanner(LapTimer);
		}
	}

	// 3. Draw Countdown Overlay at center
	if (GameState && GameState->GetRaceState() == ERaceState::PreRaceCountdown)
	{
		DrawCountdownOverlay(GameState);
	}
}

void ARacingHUD::DrawSpeedometer(float ScreenX, float ScreenY, ARacingVehiclePawn* Vehicle)
{
	if (!Vehicle) return;

	float SpeedKMH = Vehicle->GetVehicleSpeedKMH();
	int32 Gear = Vehicle->GetCurrentGear();

	FString GearStr = TEXT("N");
	if (Gear < 0)
	{
		GearStr = TEXT("R");
	}
	else if (Gear > 0)
	{
		GearStr = FString::FromInt(Gear);
	}

	// Speed box background
	DrawRect(FLinearColor(0.02f, 0.02f, 0.05f, 0.75f), ScreenX, ScreenY, 240.0f, 100.0f);

	// Gear Indicator
	FCanvasTextItem GearItem(FVector2D(ScreenX + 18.0f, ScreenY + 12.0f),
	                         FText::FromString(FString::Printf(TEXT("GEAR  %s"), *GearStr)),
	                         HUDFont, FLinearColor(1.0f, 0.8f, 0.1f));
	GearItem.Scale = FVector2D(1.6f, 1.6f);
	Canvas->DrawItem(GearItem);

	// Speed value
	FCanvasTextItem SpeedItem(FVector2D(ScreenX + 18.0f, ScreenY + 48.0f),
	                          FText::FromString(FString::Printf(TEXT("%3.0f KM/H"), SpeedKMH)),
	                          HUDFont, FLinearColor::White);
	SpeedItem.Scale = FVector2D(2.0f, 2.0f);
	Canvas->DrawItem(SpeedItem);
}

void ARacingHUD::DrawTachometer(float ScreenX, float ScreenY, ARacingVehiclePawn* Vehicle)
{
	if (!Vehicle) return;

	float RPM = Vehicle->GetEngineRPM();
	float MaxRPM = 7500.0f;
	float RPMFraction = FMath::Clamp(RPM / MaxRPM, 0.0f, 1.0f);

	float BarWidth = 240.0f;
	float BarHeight = 18.0f;

	// Background track
	DrawRect(FLinearColor(0.1f, 0.1f, 0.1f, 0.85f), ScreenX, ScreenY, BarWidth, BarHeight);

	// Filled RPM gauge
	FLinearColor BarColor = FLinearColor::Green;
	if (RPMFraction > 0.85f)
	{
		BarColor = FLinearColor::Red; // Redline alert
	}
	else if (RPMFraction > 0.65f)
	{
		BarColor = FLinearColor(1.0f, 0.6f, 0.0f); // Amber high-revs
	}

	DrawRect(BarColor, ScreenX, ScreenY, BarWidth * RPMFraction, BarHeight);
}

void ARacingHUD::DrawTimingBoard(float ScreenX, float ScreenY, ULapTimingComponent* LapTimer)
{
	if (!LapTimer) return;

	// Board background
	DrawRect(FLinearColor(0.02f, 0.02f, 0.05f, 0.75f), ScreenX, ScreenY, 320.0f, 140.0f);

	// Lap Counter
	FString LapText = FString::Printf(TEXT("LAP %d / %d"), LapTimer->GetCurrentLap(), LapTimer->GetTotalLaps());
	FCanvasTextItem LapItem(FVector2D(ScreenX + 16.0f, ScreenY + 12.0f), FText::FromString(LapText), HUDFont, FLinearColor::Yellow);
	LapItem.Scale = FVector2D(1.5f, 1.5f);
	Canvas->DrawItem(LapItem);

	// Current Lap Time
	FString CurTime = FString::Printf(TEXT("TIME:  %s"), *ULapTimingComponent::FormatLapTime(LapTimer->GetCurrentLapTime()));
	FCanvasTextItem TimeItem(FVector2D(ScreenX + 16.0f, ScreenY + 44.0f), FText::FromString(CurTime), HUDFont, FLinearColor::White);
	TimeItem.Scale = FVector2D(1.3f, 1.3f);
	Canvas->DrawItem(TimeItem);

	// Best Lap Time
	FString BestTime = FString::Printf(TEXT("BEST:  %s"), *ULapTimingComponent::FormatLapTime(LapTimer->GetBestLapTime()));
	FCanvasTextItem BestItem(FVector2D(ScreenX + 16.0f, ScreenY + 74.0f), FText::FromString(BestTime), HUDFont, FLinearColor(0.2f, 1.0f, 0.4f));
	BestItem.Scale = FVector2D(1.1f, 1.1f);
	Canvas->DrawItem(BestItem);

	// Last Lap Time
	FString LastTime = FString::Printf(TEXT("LAST:  %s"), *ULapTimingComponent::FormatLapTime(LapTimer->GetLastLapTime()));
	FCanvasTextItem LastItem(FVector2D(ScreenX + 16.0f, ScreenY + 102.0f), FText::FromString(LastTime), HUDFont, FLinearColor(0.7f, 0.7f, 0.7f));
	LastItem.Scale = FVector2D(1.1f, 1.1f);
	Canvas->DrawItem(LastItem);
}

void ARacingHUD::DrawCountdownOverlay(ARacingGameState* GameState)
{
	float Remaining = GameState->GetCountdownRemaining();
	FString TextStr;
	FLinearColor TextColor = FLinearColor::Yellow;

	if (Remaining > 2.0f)
	{
		TextStr = TEXT("3");
		TextColor = FLinearColor::Red;
	}
	else if (Remaining > 1.0f)
	{
		TextStr = TEXT("2");
		TextColor = FLinearColor(1.0f, 0.5f, 0.0f);
	}
	else if (Remaining > 0.0f)
	{
		TextStr = TEXT("1");
		TextColor = FLinearColor::Yellow;
	}
	else
	{
		TextStr = TEXT("GO!");
		TextColor = FLinearColor::Green;
	}

	float CenterX = (Canvas->ClipX / 2.0f) - 60.0f;
	float CenterY = (Canvas->ClipY / 2.0f) - 120.0f;

	FCanvasTextItem CountItem(FVector2D(CenterX, CenterY), FText::FromString(TextStr), HUDFont, TextColor);
	CountItem.Scale = FVector2D(5.0f, 5.0f);
	Canvas->DrawItem(CountItem);
}

void ARacingHUD::DrawWrongWayWarning()
{
	float CenterX = (Canvas->ClipX / 2.0f) - 180.0f;
	float TopY = 100.0f;

	// Flashing box banner
	DrawRect(FLinearColor(0.9f, 0.05f, 0.05f, 0.9f), CenterX, TopY, 360.0f, 60.0f);

	FCanvasTextItem WarningItem(FVector2D(CenterX + 25.0f, TopY + 10.0f),
	                            FText::FromString(TEXT("! WRONG WAY !")),
	                            HUDFont, FLinearColor::White);
	WarningItem.Scale = FVector2D(2.2f, 2.2f);
	Canvas->DrawItem(WarningItem);
}

void ARacingHUD::DrawFinishedBanner(ULapTimingComponent* LapTimer)
{
	float CenterX = (Canvas->ClipX / 2.0f) - 240.0f;
	float CenterY = (Canvas->ClipY / 2.0f) - 120.0f;

	DrawRect(FLinearColor(0.02f, 0.02f, 0.05f, 0.92f), CenterX, CenterY, 480.0f, 180.0f);

	FCanvasTextItem FinishItem(FVector2D(CenterX + 90.0f, CenterY + 20.0f),
	                           FText::FromString(TEXT("RACE FINISHED!")),
	                           HUDFont, FLinearColor::Yellow);
	FinishItem.Scale = FVector2D(2.4f, 2.4f);
	Canvas->DrawItem(FinishItem);

	FString BestText = FString::Printf(TEXT("Best Lap: %s"), *ULapTimingComponent::FormatLapTime(LapTimer->GetBestLapTime()));
	FCanvasTextItem BestItem(FVector2D(CenterX + 120.0f, CenterY + 85.0f),
	                         FText::FromString(BestText),
	                         HUDFont, FLinearColor::Green);
	BestItem.Scale = FVector2D(1.6f, 1.6f);
	Canvas->DrawItem(BestItem);

	FCanvasTextItem RestartHint(FVector2D(CenterX + 90.0f, CenterY + 135.0f),
	                            FText::FromString(TEXT("Press 'R' to Restart Track")),
	                            HUDFont, FLinearColor::White);
	RestartHint.Scale = FVector2D(1.2f, 1.2f);
	Canvas->DrawItem(RestartHint);
}
