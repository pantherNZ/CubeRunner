// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/PlayerController.h"
#include "BasePlayerPawn.h"

#include "CubePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class CUBERUNNER_API ACubePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACubePlayerController( const FObjectInitializer& ObjectInitializer );
};
