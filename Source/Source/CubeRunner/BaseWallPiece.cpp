// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseWallPiece.h"
#include "CubeRunner.h"
#include "Components/BoxComponent.h"

// Sets default values
ABaseWallPiece::ABaseWallPiece( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, LeftWall( true )
	, ScaleOffset( 15.0f, 15.0f, 15.0f )
	, PositionOffset( 0.0f, 0.0f, 0.0f )
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Setup components
	Root = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
	RootComponent = Root;

	WallMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Floor Mesh" ) );
	WallMesh->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );

	WallCollision = CreateDefaultSubobject<UBoxComponent>( TEXT( "Spawn Collision" ) );
	WallCollision->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	WallCollision->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	WallCollision->bGenerateOverlapEvents = true;
}

// Called when the game starts or when spawned
void ABaseWallPiece::BeginPlay()
{
	Super::BeginPlay();
}

void ABaseWallPiece::OnConstruction( const FTransform& Transform )
{
	if( WallMesh && WallCollision )
	{
		auto NewTransform = WallMesh->GetComponentTransform();
		NewTransform.SetScale3D( NewTransform.GetScale3D() + ScaleOffset );
		NewTransform.SetLocation( NewTransform.GetLocation() + PositionOffset + WallMesh->GetUpVector() * WallMesh->Bounds.BoxExtent.Z );
		WallCollision->SetWorldTransform( NewTransform );
	}
}