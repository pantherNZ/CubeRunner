// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseTurnFloorPiece.h"
#include "CubeRunner.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"

ABaseTurnFloorPiece::ABaseTurnFloorPiece( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	TurnZone = CreateDefaultSubobject<UBoxComponent>( TEXT( "Turn Zone" ) );
	TurnZone->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	TurnZone->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	TurnZone->bGenerateOverlapEvents = true;

	TurnStartPoint = CreateDefaultSubobject<UArrowComponent>( TEXT( "Turn Start Point" ) );
	TurnStartPoint->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	TurnStartPoint->SetWorldScale3D( FVector( 20.0f, 20.0f, 20.0f ) );

	TurnEndPoint = CreateDefaultSubobject<UArrowComponent>( TEXT( "Turn End Point" ) );
	TurnEndPoint->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	TurnEndPoint->SetWorldScale3D( FVector( 20.0f, 20.0f, 20.0f ) );

	BezierInterpolation = 0.0f;
}

void ABaseTurnFloorPiece::BeginTurn( FVector Start, float CurrentPlayerSpeed )
{
	TurnStartPosition = Start;
	CalculateCurveData();

	PlayerSpeed = CurrentPlayerSpeed;
	InterpSpeed = PlayerSpeed / CalculateBezierCurveLengthSimple();
}

void ABaseTurnFloorPiece::CalculateCurveData()
{
	auto TurnStartPointPos = TurnStartPoint->GetComponentLocation();
	TurnStartPointPos.Z = TurnStartPosition.Z;

	auto TurnEndPointPos = TurnEndPoint->GetComponentLocation();
	TurnEndPointPos.Z = TurnStartPosition.Z;

	const auto DirectionFromCentre = TurnStartPosition - TurnStartPointPos;
	const auto DistFromCentre = FVector::DotProduct( DirectionFromCentre, TurnStartPoint->GetRightVector() );
	TurnEndPosition = TurnEndPointPos + TurnEndPoint->GetRightVector() * DistFromCentre;

	const auto DirectionFromEnd = TurnEndPosition - TurnStartPosition;
	const auto DistFromStart = FVector::DotProduct( DirectionFromEnd, TurnStartPoint->GetForwardVector() );
	TurnCornerPosition = TurnStartPosition + TurnStartPoint->GetForwardVector() * DistFromStart;
}

FTransform ABaseTurnFloorPiece::GetTurnTargetTransform( float Offset /*= 0.0f*/ )
{
	BezierInterpolation += InterpSpeed * GetWorld()->GetDeltaSeconds();
	return GetTurnTargetTransformInternal( BezierInterpolation, Offset );
}

FTransform ABaseTurnFloorPiece::GetTurnTargetTransformInternal( float Interpolation, float Offset /*= 0.0f*/ )
{
	if( Offset != 0.0f )
	{
		TurnStartPosition += TurnStartPoint->GetRightVector() * Offset;
		CalculateCurveData();
		InterpSpeed = PlayerSpeed / CalculateBezierCurveLengthSimple();
	}

	TArray< FVector > ControlPoints;
	ControlPoints.Add( TurnStartPosition );
	ControlPoints.Add( TurnCornerPosition );
	ControlPoints.Add( TurnEndPosition );

	TargetTransform.SetLocation( Bezier( Interpolation, ControlPoints ) );
	auto NextTargetPosition = Bezier( Interpolation + 0.01f, ControlPoints );

	auto TargetDirection = NextTargetPosition - TargetTransform.GetLocation();
	TargetDirection.Normalize();
	TargetTransform.SetRotation( FRotationMatrix::MakeFromX( TargetDirection ).ToQuat() );

	return TargetTransform;
}

float ABaseTurnFloorPiece::CalculateBezierCurveLengthSimple()
{
	// Radius calculation
	const auto DirectionFromEnd = TurnEndPosition - TurnStartPosition;
	const auto DistFromStart = FVector::DotProduct( DirectionFromEnd, TurnEndPoint->GetForwardVector() );

	// Circumference = 2 * pi * R
	return ( 2.0f * PI * DistFromStart ) / 4.0f;
}