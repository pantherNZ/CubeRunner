// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseObstacleComponent.h"
#include "CubeRunner.h"
#include "BaseFloorPiece.h"
#include "CubeSingletonDataLibrary.h"

// Sets default values
UBaseObstacleComponent::UBaseObstacleComponent( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, WaypointIndex( 0 )
	, MovementSpeed( 0.0f )
	, RotateTowardsTarget( false )
	, MinRequiredDistanceToWaypoint( 5.0f )
	, MovementStyle( EMovementStyle::EMS_NONE )
	, ReplaceWithCorrectEdgeObstacle( false )
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseObstacleComponent::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetComponentLocation();

	if( ReplaceWithCorrectEdgeObstacle )
	{
		ReplaceWithCorrectEdgeObstacle = false;

		if( auto* owner = Cast< ABaseFloorPiece >( GetOwner() ) )
		{
			owner->SpawnObstacleMultiVariations( owner->FindObstacleClass(), GetComponentTransform(), Variations );
			DestroyComponent();
		}
		else
			UCubeSingletonDataLibrary::CustomLog( "Obstacle owner not valid with ReplaceWithCorrectEdgeObstacle feature", LogDisplayType::Error );
	}
}

void UBaseObstacleComponent::TickComponent( float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	if( ( WaypointPositions.Num() || WaypointTargets.Num() ) && MovementStyle > EMovementStyle::EMS_PHYSICS )
	{
		const auto TargetPos = ( WaypointPositions.Num() ? WaypointPositions[ WaypointIndex ] : WaypointTargets[ WaypointIndex ]->GetComponentLocation() );
		auto Direction = TargetPos - GetComponentLocation();

		// Distance to current target
		if( FVector::DistSquared( TargetPos, GetComponentLocation() ) <= ( MinRequiredDistanceToWaypoint * MinRequiredDistanceToWaypoint ) )
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
			SetWorldLocation( GetComponentLocation() + Direction * DeltaTime * MovementSpeed );
			break;
		}
		case EMovementStyle::EMS_CUBIC_INTERP:
		{
			SetWorldLocation( GetComponentLocation() + Direction / 100.0f * DeltaTime * MovementSpeed );
			break;
		}
		default: break;
		}
	}
	else if( MovementStyle == EMovementStyle::EMS_CIRCLE )
	{
		const auto TotalTime = GetWorld()->GetTimeSeconds() * MovementSpeed;
		SetWorldLocation( StartLocation + FVector( FMath::Sin( TotalTime ), FMath::Cos( TotalTime ), 0.0f ) * CircleMovementRadius );
	}
}

void UBaseObstacleComponent::AddDynamicWaypoint( const USceneComponent* Marker )
{
	if( Marker->IsValidLowLevel() && !Marker->IsPendingKill() )
		WaypointPositions.Add( Marker->GetComponentLocation() );
}

void UBaseObstacleComponent::AddDynamicWaypointLocation( const FVector Location )
{
	WaypointPositions.Add( Location );
}

void UBaseObstacleComponent::AddDynamicWaypoints( const USceneComponent* Marker, const USceneComponent* Marker2 )
{
	AddDynamicWaypoint( Marker );
	AddDynamicWaypoint( Marker2 );
}

void UBaseObstacleComponent::AddDynamicWaypointsLocation( const FVector Location, const FVector Location2 )
{
	AddDynamicWaypointLocation( Location );
	AddDynamicWaypointLocation( Location2 );
}
