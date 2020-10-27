// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/staticMeshComponent.h"
#include "BaseObstacle.h"
#include "BaseObstacleComponent.generated.h"

UCLASS(Blueprintable)
class CUBERUNNER_API UBaseObstacleComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

	// Functions
public:
	UBaseObstacleComponent( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypoint( const USceneComponent* Marker );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypointLocation( const FVector Location );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypoints( const USceneComponent* Marker, const USceneComponent* Marker2 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypointsLocation( const FVector Location, const FVector Location2 );

	// Members
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 WaypointIndex;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< FVector > WaypointPositions;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< class USceneComponent* > WaypointTargets; // This isn't supported yet apparently =(
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float MovementSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool RotateTowardsTarget;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float MinRequiredDistanceToWaypoint;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) EMovementStyle MovementStyle;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< int32 > Variations;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 CircleMovementRadius;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool ReplaceWithCorrectEdgeObstacle;

private:
	FVector StartLocation;
};
