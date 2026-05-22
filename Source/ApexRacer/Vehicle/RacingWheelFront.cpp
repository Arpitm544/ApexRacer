#include "Vehicle/RacingWheelFront.h"

URacingWheelFront::URacingWheelFront()
{
	AxleType = EAxleType::Front;
	WheelRadius = 38.0f;
	WheelWidth = 25.0f;
	bAffectedBySteering = true;
	MaxSteerAngle = 38.0f;
	bAffectedByHandbrake = false;
	bAffectedByEngine = false; // RWD configuration (set to true for AWD)
	bAffectedByBrake = true;

	// Suspension settings for responsive racing handling
	SuspensionMaxRaise = 10.0f;
	SuspensionMaxDrop = 12.0f;
	SuspensionDampingRatio = 0.85f;
	SpringRate = 250.0f;
	SpringPreload = 50.0f;

	// Friction and lateral stiffness
	LateralSlipGraph.Reset();
	LateralSlipGraph.AddKey(0.0f, 0.0f);
	LateralSlipGraph.AddKey(3.0f, 0.95f);
	LateralSlipGraph.AddKey(12.0f, 1.15f);
	LateralSlipGraph.AddKey(25.0f, 0.85f);
}
