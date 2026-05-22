#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "RacingWheelRear.generated.h"

/**
 * Rear wheel configuration for the racing vehicle.
 * Configured as the primary drive axle with handbrake engagement.
 */
UCLASS()
class APEXRACER_API URacingWheelRear : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	URacingWheelRear();
};
