#include "Track/TrackSplineActor.h"

ATrackSplineActor::ATrackSplineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TrackSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrackSpline"));
	RootComponent = TrackSpline;
	TrackSpline->SetClosedLoop(true);
}

float ATrackSplineActor::GetTotalTrackLength() const
{
	if (TrackSpline)
	{
		return TrackSpline->GetSplineLength();
	}
	return 0.0f;
}

float ATrackSplineActor::GetDistanceAlongSpline(const FVector& WorldLocation) const
{
	if (!TrackSpline)
	{
		return 0.0f;
	}

	float InputKey = TrackSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
	return TrackSpline->GetDistanceAlongSplineAtSplineInputKey(InputKey);
}

float ATrackSplineActor::GetTrackProgressFraction(const FVector& WorldLocation) const
{
	float TotalLength = GetTotalTrackLength();
	if (TotalLength <= KINDA_SMALL_NUMBER)
	{
		return 0.0f;
	}

	float CurrentDistance = GetDistanceAlongSpline(WorldLocation);
	return FMath::Clamp(CurrentDistance / TotalLength, 0.0f, 1.0f);
}

FVector ATrackSplineActor::GetDirectionAtWorldLocation(const FVector& WorldLocation) const
{
	if (!TrackSpline)
	{
		return FVector::ForwardVector;
	}

	float InputKey = TrackSpline->FindInputKeyClosestToWorldLocation(WorldLocation);
	return TrackSpline->GetDirectionAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
}
