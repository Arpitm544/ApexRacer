#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UArrowComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointTriggeredSignature, ACheckpointActor*, Checkpoint, AActor*, OtherActor, bool, bValidDirection);

/**
 * Checkpoint Gate Actor placed along the racing track.
 * Enforces sequential crossing and correct driving direction to prevent shortcutting.
 */
UCLASS()
class APEXRACER_API ACheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointActor();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Checkpoint")
	int32 CheckpointIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Racing|Checkpoint")
	bool bIsFinishLine = false;

	UPROPERTY(BlueprintAssignable, Category = "Racing|Events")
	FOnCheckpointTriggeredSignature OnCheckpointTriggered;

	// Checks if vehicle is traversing forward through gate (Dot product > 0)
	UFUNCTION(BlueprintPure, Category = "Racing|Checkpoint")
	bool IsPassingInCorrectDirection(const FVector& VehicleVelocity) const;

	UFUNCTION(BlueprintCallable, Category = "Racing|Checkpoint")
	FTransform GetSpawnTransform() const;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                    bool bFromSweep, const FHitResult& SweepResult);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UArrowComponent> ForwardDirectionArrow;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UStaticMeshComponent> LeftPillarMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Racing|Components")
	TObjectPtr<UStaticMeshComponent> RightPillarMesh;
};
