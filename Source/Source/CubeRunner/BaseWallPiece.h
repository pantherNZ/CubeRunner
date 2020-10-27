// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseWallPiece.generated.h"

UCLASS()
class CUBERUNNER_API ABaseWallPiece : public AActor
{
	GENERATED_BODY()
	
	// Functions
public:	
	ABaseWallPiece( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void OnConstruction( const FTransform& Transform ) override;

protected:

private:

	// Members
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class USceneComponent* Root;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UStaticMeshComponent* WallMesh;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UBoxComponent* WallCollision;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool LeftWall;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) FVector ScaleOffset;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) FVector PositionOffset;

protected:

private:

};
