// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BaseFloorPiece.h"
#include "BaseTransitionFloorPiece.generated.h"

UCLASS()
class CUBERUNNER_API ABaseTransitionFloorPiece : public ABaseFloorPiece
{
	GENERATED_BODY()
	
	// Functions
public:
	ABaseTransitionFloorPiece( const FObjectInitializer& ObjectInitializer );
};
