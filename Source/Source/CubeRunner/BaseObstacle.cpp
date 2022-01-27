// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseObstacle.h"
#include "CubeRunner.h"

// Sets default values
ABaseObstacle::ABaseObstacle( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, WaypointIndex( 0 )
	, MovementSpeed( 0.0f )
	, RotateTowardsTarget( false )
	, MovementStyle( EMovementStyle::EMS_NONE )
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseObstacle::Tick( float DeltaSeconds )
{
	Super::Tick( DeltaSeconds );

	if( WaypointPositions.Num() && MovementStyle > EMovementStyle::EMS_PHYSICS )
	{
		const auto TargetPos = WaypointPositions[ WaypointIndex ];
		auto Direction = TargetPos - GetActorLocation();

		// Distance to current target
		if( FVector::DistSquared( TargetPos, GetActorLocation() ) <= 5.0f )
		{
			WaypointIndex++;

			if( WaypointIndex >= WaypointPositions.Num() )
				WaypointIndex = 0;
		}

		// Movement
		switch( MovementStyle )
		{
		case EMovementStyle::EMS_LERP:
		{
			Direction.Normalize();
			SetActorLocation( GetActorLocation() + Direction * DeltaSeconds * MovementSpeed );
			break;
		}

		case EMovementStyle::EMS_CUBIC_INTERP:
		{
			SetActorLocation( GetActorLocation() + Direction / 100.0f * DeltaSeconds * MovementSpeed );
			break;
		}

		default: break;
		}
	}
}

void ABaseObstacle::SetDynamicObstacle( const float NewMovementSpeed, EMovementStyle NewMovementStyle )
{
	MovementSpeed = NewMovementSpeed;
	MovementStyle = NewMovementStyle;
}

void ABaseObstacle::AddDynamicWaypoint( const UChildActorComponent* Marker )
{
	if( IsValid( Marker ) )
		WaypointPositions.Add( Marker->GetComponentLocation() );
}

void ABaseObstacle::AddDynamicWaypointLocation( const FVector Location )
{
	WaypointPositions.Add( Location );
}

void ABaseObstacle::AddDynamicWaypoints( const UChildActorComponent* Marker, const UChildActorComponent* Marker2 )
{
	AddDynamicWaypoint( Marker );
	AddDynamicWaypoint( Marker2 );
}

void ABaseObstacle::AddDynamicWaypointsLocation( const FVector Location, const FVector Location2 )
{
	AddDynamicWaypointLocation( Location );
	AddDynamicWaypointLocation( Location2 );
}
