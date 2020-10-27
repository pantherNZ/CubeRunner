// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseObstacle.generated.h"

UENUM( BlueprintType )
enum class EMovementStyle : uint8
{
	EMS_NONE UMETA( DisplayName = "No Movement" ),
	EMS_CIRCLE UMETA( DisplayName = "Circular" ),
	EMS_PHYSICS UMETA( DisplayName = "Physics Enabled" ),
	EMS_LERP UMETA( DisplayName = "Linear Interpolation" ),
	EMS_CUBIC_INTERP UMETA( DisplayName = "Cubic Interpolation" ),
};

UCLASS()
class CUBERUNNER_API ABaseObstacle : public AActor
{
	GENERATED_BODY()
	
	// Functions
public:
	ABaseObstacle( const FObjectInitializer& ObjectInitializer );

	virtual void Tick( float DeltaSeconds ) override;

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SetDynamicObstacle( const float MovementSpeed, EMovementStyle MovementStyle );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypoint( const UChildActorComponent* Marker );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypointLocation( const FVector Location );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypoints( const UChildActorComponent* Marker, const UChildActorComponent* Marker2 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddDynamicWaypointsLocation( const FVector Location, const FVector Location2 );

	// Members
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 WaypointIndex;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< FVector > WaypointPositions;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float MovementSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool RotateTowardsTarget;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) EMovementStyle MovementStyle;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 Variation;
};
