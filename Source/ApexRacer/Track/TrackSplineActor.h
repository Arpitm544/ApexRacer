#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SplineComponent.h"
#include "TrackSplineActor.generated.h"

class ACheckpointActor;

/**
 * Spline representation of the race track circuit.
 * Calculates vehicle progression percentage and distance around the circuit.
 */
UCLASS()
class APEXRACER_API ATrackSplineActor : public AActor
{
	GENERATED_BODY()

public:
	ATrackSplineActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Track")
	TObjectPtr<USplineComponent> TrackSpline;

	// Total length in centimeters of the track circuit
	UFUNCTION(BlueprintPure, Category = "Racing|Track")
	float GetTotalTrackLength() const;

	// Calculates linear distance traveled along the spline circuit for a world location
	UFUNCTION(BlueprintPure, Category = "Racing|Track")
	float GetDistanceAlongSpline(const FVector& WorldLocation) const;

	// Returns normalized progress [0.0 - 1.0] along the circuit
	UFUNCTION(BlueprintPure, Category = "Racing|Track")
	float GetTrackProgressFraction(const FVector& WorldLocation) const;

	// Finds nearest direction tangent vector at world position
	UFUNCTION(BlueprintPure, Category = "Racing|Track")
	FVector GetDirectionAtWorldLocation(const FVector& WorldLocation) const;
};
