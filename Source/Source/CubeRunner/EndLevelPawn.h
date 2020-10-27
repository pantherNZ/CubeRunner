// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "EndLevelPawn.generated.h"

UCLASS()
class CUBERUNNER_API AEndLevelPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AEndLevelPawn( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	
	// Members
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class USceneComponent* Root;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UCameraComponent* Camera;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float TotalDistanceTravelled;
};
