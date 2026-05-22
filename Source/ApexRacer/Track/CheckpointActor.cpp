#include "Track/CheckpointActor.h"
#include "ApexRacer.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Vehicle/RacingVehiclePawn.h"

ACheckpointActor::ACheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	ForwardDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("ForwardDirectionArrow"));
	ForwardDirectionArrow->SetupAttachment(RootComponent);
	ForwardDirectionArrow->ArrowSize = 2.0f;
	ForwardDirectionArrow->ArrowColor = FColor::Green;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(100.0f, 800.0f, 300.0f)); // Wide track gate
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);

	LeftPillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftPillarMesh"));
	LeftPillarMesh->SetupAttachment(RootComponent);
	LeftPillarMesh->SetRelativeLocation(FVector(0.0f, -800.0f, 150.0f));
	LeftPillarMesh->SetCollisionProfileName(TEXT("BlockAll"));

	RightPillarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightPillarMesh"));
	RightPillarMesh->SetupAttachment(RootComponent);
	RightPillarMesh->SetRelativeLocation(FVector(0.0f, 800.0f, 150.0f));
	RightPillarMesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void ACheckpointActor::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointActor::OnOverlapBegin);
}

bool ACheckpointActor::IsPassingInCorrectDirection(const FVector& VehicleVelocity) const
{
	if (VehicleVelocity.IsNearlyZero(10.0f))
	{
		return true;
	}

	FVector GateForward = GetActorForwardVector();
	float Dot = FVector::DotProduct(GateForward, VehicleVelocity.GetSafeNormal());
	return Dot > 0.1f; // Must have forward component through gate
}

FTransform ACheckpointActor::GetSpawnTransform() const
{
	FVector SpawnLocation = GetActorLocation() + (GetActorForwardVector() * 200.0f) + FVector(0.0f, 0.0f, 30.0f);
	return FTransform(GetActorRotation(), SpawnLocation, FVector::OneVector);
}

void ACheckpointActor::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                     bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	// Verify the overlapping actor is a racing pawn
	if (ARacingVehiclePawn* Vehicle = Cast<ARacingVehiclePawn>(OtherActor))
	{
		bool bCorrectDir = IsPassingInCorrectDirection(Vehicle->GetVelocity());
		UE_LOG(LogApexRacer, Log, TEXT("Checkpoint %d crossed by %s (Valid Dir: %s)"),
		       CheckpointIndex, *Vehicle->GetName(), bCorrectDir ? TEXT("YES") : TEXT("NO"));

		OnCheckpointTriggered.Broadcast(this, Vehicle, bCorrectDir);
	}
}
