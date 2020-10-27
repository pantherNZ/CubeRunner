// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseFloorPiece.h"
#include "Components/ArrowComponent.h"
#include "BaseTurnFloorPiece.generated.h"

UCLASS()
class CUBERUNNER_API ABaseTurnFloorPiece : public ABaseFloorPiece
{
	GENERATED_BODY()

	// Functions
public:
	ABaseTurnFloorPiece( const FObjectInitializer& ObjectInitializer );

	UFUNCTION( BlueprintCallable, Category = "TurnPiece" )
	FTransform GetTurnTargetTransform( float Offset = 0.0f );

	UFUNCTION( BlueprintCallable, Category = "TurnPiece" )
	void BeginTurn( FVector Start, float PlayerSpeed );

	UFUNCTION( BlueprintCallable, Category = "TurnPiece" )
	FTransform GetTurnTargetTransformInternal( float Interpolation, float Offset = 0.0f );

	UFUNCTION( BlueprintCallable, Category = "TurnPiece" )
	float CalculateBezierCurveLengthSimple();

	UFUNCTION( BlueprintCallable, Category = "TurnPiece" )
	void CalculateCurveData();

	// Members
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UBoxComponent* TurnZone;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UArrowComponent* TurnStartPoint;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UArrowComponent* TurnEndPoint;

private:
	FVector TurnStartPosition;
	FVector TurnEndPosition;
	FVector TurnCornerPosition;
	float BezierInterpolation;
	float InterpSpeed;
	float PlayerSpeed;
	FTransform TargetTransform;
};
