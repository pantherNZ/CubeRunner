// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseRandomisedFloorPiece.h"
#include "CubeRunner.h"
#include "BasePlayerPawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "CubeRunnerGameMode.h"
#include "CubeSingletonDataLibrary.h"

#include <random>

#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

ABaseRandomisedFloorPiece::ABaseRandomisedFloorPiece( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, DensityVariation( 30 )
	, HorizontalUpdateWidth( 400.0f )
	, MoveThresholdMin( 0.0f )
	, MoveThresholdMax( 0.0f )
{
	EndCollision = CreateDefaultSubobject< UBoxComponent >( TEXT( "End Collision" ) );
	EndCollision->AttachToComponent( FloorMesh, FAttachmentTransformRules::KeepRelativeTransform );
	EndCollision->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	EndCollision->SetGenerateOverlapEvents( true );
}

void ABaseRandomisedFloorPiece::BeginPlay()
{
	Super::BeginPlay();

	EndCollision->OnComponentEndOverlap.AddDynamic( this, &ABaseRandomisedFloorPiece::OnEndCollisionOverlapEnd );
}

void ABaseRandomisedFloorPiece::FloorPieceBeginPlay()
{
	const auto CubeGM = Cast< ACubeRunnerGameMode >( GetWorld()->GetAuthGameMode() );
	const float fDensityBase = ( float )CubeGM->LevelRandomisedFloorPieceDensity;
	const float fDensityVar = ( float )DensityVariation;

	std::default_random_engine generator;
	std::normal_distribution< float > distribution( fDensityBase, fDensityVar );

	// Spawn obstacles
	Density = ( int32 )distribution( generator ); 
	SpawnObstacle( FloorMesh->GetComponentLocation(), FloorMesh->Bounds.BoxExtent, Density );

	Super::FloorPieceBeginPlay();
}

void ABaseRandomisedFloorPiece::SpawnObstacle( FVector Origin, FVector BoxExtent, int32 _Density )
{
	FRotator Rotation = FloorMesh->GetUpVector().Rotation();

	for( int32 i = 0; i < _Density; ++i )
	{
		auto RandPos = UKismetMathLibrary::RandomPointInBoundingBox( Origin, FVector( BoxExtent.X - 60.0f, BoxExtent.Y - 60.0f, 0.0f ) );
		RandPos.Z = FloorMesh->GetComponentLocation().Z + 75.0f;
		FTransform transform( Rotation, RandPos, FVector( 1.0f, 1.0f, 1.0f ) );
		Super::SpawnObstacle( UCubeSingletonDataLibrary::GetGameData()->ClassicCubeObstacleBPClass, transform, 0 );
	}
}

void ABaseRandomisedFloorPiece::MoveFloor( FVector Offset, float DistanceMoved )
{
	FVector Difference = ConnectionPoint->GetComponentLocation() - FloorMesh->GetComponentLocation();
	FloorMesh->SetWorldLocation( Offset );
	ConnectionPoint->SetWorldLocation( Offset + Difference );
	float DistDiff = FMath::Clamp( DistanceMoved, MoveThresholdMin - HorizontalUpdateWidth,  MoveThresholdMax + HorizontalUpdateWidth );

	// Density & extent
	const auto NewDensity = HorizontalUpdateWidth * ( Density / FloorMesh->Bounds.BoxExtent.Y );
	const auto FinalDensity = FMath::RoundToInt( FMath::FRandRange( NewDensity / 2.0f, NewDensity ) );
	const auto Extent = FVector( FloorMesh->Bounds.BoxExtent.X, HorizontalUpdateWidth / 2.0f, 0.0f );

	// Helper lambdas
	const auto GetOrigin = [&]( bool MinOrigin ) -> FVector
	{
		FVector SideOffset = GetActorRightVector();

		if( MinOrigin )
			SideOffset *= ( ( -FloorMesh->Bounds.BoxExtent.Y + HorizontalUpdateWidth / 2.0f  ) - MoveThresholdMin );
		else
			SideOffset *= ( ( FloorMesh->Bounds.BoxExtent.Y - HorizontalUpdateWidth / 2.0f ) + MoveThresholdMax );

		return GetActorLocation() + GetActorForwardVector() * FloorMesh->Bounds.BoxExtent.X + SideOffset;
	};
	return;
	if( DistDiff <= MoveThresholdMin - HorizontalUpdateWidth )
	{
		MoveThresholdMin -= HorizontalUpdateWidth;		
		SpawnObstacle( GetOrigin( true ), Extent, FinalDensity );
	}
	else if( DistDiff >= MoveThresholdMax + HorizontalUpdateWidth )
	{
		MoveThresholdMax += HorizontalUpdateWidth;
		SpawnObstacle( GetOrigin( false ), Extent, FinalDensity );
	}
}

void ABaseRandomisedFloorPiece::OnEndCollisionOverlapEnd( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
	if( IsValid( Cast< ABasePlayerPawn >( OtherActor ) ) )
	{
		const auto CubeGM = Cast< ACubeRunnerGameMode >( GetWorld()->GetAuthGameMode() );
		CubeGM->UpdateNewFloorPiecePosition = false;
	}
}