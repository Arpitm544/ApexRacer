#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "RacingWheelFront.generated.h"

/**
 * Front wheel configuration for the racing vehicle.
 * Configured with steering angle, front suspension stiffness, and ABS support.
 */
UCLASS()
class APEXRACER_API URacingWheelFront : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	URacingWheelFront();
};
