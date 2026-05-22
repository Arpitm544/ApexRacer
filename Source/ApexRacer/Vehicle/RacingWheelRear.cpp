#include "Vehicle/RacingWheelRear.h"

URacingWheelRear::URacingWheelRear()
{
	AxleType = EAxleType::Rear;
	WheelRadius = 38.0f;
	WheelWidth = 28.0f; // Wider rear tire profile for traction
	bAffectedBySteering = false;
	MaxSteerAngle = 0.0f;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true; // Primary drive axle
	bAffectedByBrake = true;

	// Stiffer rear suspension for stability under acceleration
	SuspensionMaxRaise = 10.0f;
	SuspensionMaxDrop = 12.0f;
	SuspensionDampingRatio = 0.90f;
	SpringRate = 280.0f;
	SpringPreload = 55.0f;

	// Lateral traction curve
	LateralSlipGraph.Reset();
	LateralSlipGraph.AddKey(0.0f, 0.0f);
	LateralSlipGraph.AddKey(4.0f, 1.05f);
	LateralSlipGraph.AddKey(14.0f, 1.20f);
	LateralSlipGraph.AddKey(28.0f, 0.90f);
}
