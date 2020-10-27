// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseFloorPiece.h"
#include "BaseRandomisedFloorPiece.generated.h"

UCLASS()
class CUBERUNNER_API ABaseRandomisedFloorPiece : public ABaseFloorPiece
{
	GENERATED_BODY()
	
	// Functions
public:
	ABaseRandomisedFloorPiece( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void FloorPieceBeginPlay() override;

	void SpawnObstacle( FVector Origin, FVector BoxExtent, int32 _Density );
	void MoveFloor( FVector Offset, float DistanceMoved );

	UFUNCTION()		
	void OnEndCollisionOverlapEnd( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );

protected:

	// Members
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UBoxComponent* EndCollision;

	UPROPERTY( EditAnywhere, Category = "Stats" ) int32 DensityVariation;
	UPROPERTY( EditAnywhere, Category = "Stats" ) float HorizontalUpdateWidth;

private:
	float MoveThresholdMin;
	float MoveThresholdMax;
	int32 Density;
};
