// Fill out your copyright notice in the Description page of Project Settings.

#include "EndLevelPawn.h"
#include "CubeRunner.h"

// Constructor
AEndLevelPawn::AEndLevelPawn( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, TotalDistanceTravelled( 0.0f )
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup components
	Root = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
	RootComponent = Root;

	Camera = CreateDefaultSubobject<UCameraComponent>( TEXT( "Camera" ) );
	Camera->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
}

// Called when the game starts or when spawned
void AEndLevelPawn::BeginPlay()
{
	Super::BeginPlay();
	
}