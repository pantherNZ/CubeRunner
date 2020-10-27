// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseAIPawn.h"
#include "CubeRunner.h"

#include <algorithm>

// Sets default values
ABaseAIPawn::ABaseAIPawn( const class FObjectInitializer& ObjectInitializer ) 
	: Super( ObjectInitializer )
	, GridSize( 50 )
	, MinimumPathDistance( 500 )
	, MaximumPathDistance( 500 )
{

}

void ABaseAIPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseAIPawn::Tick( float DeltaSeconds )
{
	Super::Tick( DeltaSeconds );
	
	ProcessPathFinding( DeltaSeconds );
	ProcessAIMovement( DeltaSeconds );
}

void ABaseAIPawn::SetupPlayerInputComponent( class UInputComponent* NewInputComponent )
{
	Super::SetupPlayerInputComponent( NewInputComponent );
}

void ABaseAIPawn::ProcessPathFinding( float DeltaSeconds )
{
	// Continuously running, basic search algorithm
	if( FVector::DotProduct( CurrentPath.back() - GetActorLocation( ), GetActorForwardVector( ) ) < MinimumPathDistance )
	{
		std::vector< PathNode* > OpenList;
		std::vector< PathNode* > ClosedList;

		// Start / continue from position
		FVector StartPosition = ( CurrentPath.size( ) == 0 ? GetActorLocation( ) : CurrentPath.back( ) );
		PathNode* Start = new PathNode( StartPosition );
		OpenList.push_back( Start );

		while( OpenList.size( ) > 0 )
		{
			// Randomly pick a path (could add weightings to move forward more etc..)
			int32 Index = FMath::RandRange( 0, OpenList.size( ) - 1 );
			PathNode* Current = OpenList[Index];

			// Check if we have pathed far enough
			if( FVector::DotProduct( Current->Location - GetActorLocation( ), GetActorForwardVector( ) ) >= MaximumPathDistance )
			{
				ReconstructPath( Current );
				return;
			}

			OpenList.erase( OpenList.begin( ) + Index );
			ClosedList.push_back( Current );

			// For each valid "neighbour"
			// TODO: Add left / right neighbours enough to fit the hypothetical maximum strafing position for each side
			PathNode* Neighbours[3];
			Neighbours[0] = new PathNode( Current->Location + GetActorForwardVector( ) * GridSize );
			Neighbours[1] = new PathNode( Current->Location + GetActorForwardVector( ) * GridSize + GetActorRightVector( ) * GridSize );
			Neighbours[2] = new PathNode( Current->Location + GetActorForwardVector( ) * GridSize - GetActorRightVector( ) * GridSize );

			for( int32 i = 0; i < 3; ++i )
			{
				PathNode* CurrentNeighbour = Neighbours[i];

				// Ignore if it has already been evaluated
				if( std::find_if( ClosedList.begin( ), ClosedList.end( ), [&CurrentNeighbour]( PathNode* Node )
				{
					return Node->Location == CurrentNeighbour->Location;
				} ) != ClosedList.end( ) )
					continue;

				// Ignore if it is already in the open list
				if( std::find_if( OpenList.begin( ), OpenList.end( ), [&CurrentNeighbour]( PathNode* Node )
				{
					return Node->Location == CurrentNeighbour->Location;
				} ) != OpenList.end( ) )
					continue;

				// Not a valid location!
				if( CheckLocationCollision( CurrentNeighbour->Location) )
					continue;

				CurrentNeighbour->CameFrom = Current;
				OpenList.push_back( CurrentNeighbour );
			}
		}
	}
}

void ABaseAIPawn::ReconstructPath( PathNode* FinalNode )
{
	PathNode* Current = FinalNode;
	std::vector< FVector > Path;

	// Traverse each node from end to find where we came from and create final path
	while( Current != nullptr )
	{
		Path.push_back( Current->Location );
		Current = Current->CameFrom;
	}

	// Reverse as the path was traced from last to first
	std::reverse( Path.begin( ), Path.end( ) );

	// Add to our actual AI path
	CurrentPath.insert( CurrentPath.end( ), Path.begin( ), Path.end( ) );
}

void ABaseAIPawn::ProcessAIMovement( float DeltaSeconds )
{
	
}

bool ABaseAIPawn::CheckLocationCollision( FVector Location )
{
	FCollisionQueryParams TraceParams( FName( TEXT( "AI Pathfinding Collision Trace" ) ), true, nullptr );
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnPhysicalMaterial = false;

	FHitResult HitResult;

//	GetWorld( )->LineTraceSingle( HitResult, Location, Location, ECC_Visible, TraceParams );

	return ( HitResult.GetActor( ) != NULL );
}