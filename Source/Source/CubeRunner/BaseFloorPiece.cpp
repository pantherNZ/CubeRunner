// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseFloorPiece.h"
#include "CubeRunner.h"
#include "BasePlayerPawn.h"
#include "CubeRunnerGameMode.h"
#include "Components/ArrowComponent.h"
#include "BaseTransitionFloorPiece.h"
#include <vector>
#include "BaseObstacle.h"
#include "Kismet/KismetMathLibrary.h"
#include "CubeDataSingleton.h"
#include "BaseObstacleComponent.h"
#include "CubeGameInstance.h"
#include "CubeSingletonDataLibrary.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// Static data
namespace
{
	bool InstancedObstacleSpawningEnabled = true;
	float ObstacleSpawnTraceHeight = 3000.0f;
}

TArray< TArray < int32 > > ABaseFloorPiece::BinomialLookUpTable;

// Sets default values
ABaseFloorPiece::ABaseFloorPiece( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, CoolDownCounter( 0 )
	, CoolDownLength( 0 )
	, PieceFamily( EPieceFamily::EPF_RANDOMISED )
	, UpgradeActor( nullptr )
	, TransitionPiece( false )
	, ConstructionScriptRun( false )
	, Variation( 0 )
	, MaxVariationClassic( 0 )
	, MaxVariationAdvanced( 0 )
	, InstancedObstacleData( TMap< UStaticMesh*, FInstancedObstacleDataContainer >() )
	, SpawnedChildObstacles( TArray< FChildObstacle >() )
	, HasTriggered( false )
	, EndLevelPiece( false )
{
	PrimaryActorTick.bCanEverTick = true;

	// Setup components
	Root = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
	RootComponent = Root;

	RootDir = CreateDefaultSubobject<UArrowComponent>( TEXT( "Root Direction" ) );
	RootDir->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	RootDir->SetWorldScale3D( FVector( 20.0f, 20.0f, 20.0f ) );

	FloorMesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Floor Mesh" ) );
	FloorMesh->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );

	ConnectionPoint = CreateDefaultSubobject<UArrowComponent>( TEXT( "Connection Point" ) );
	ConnectionPoint->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	ConnectionPoint->SetWorldScale3D( FVector( 20.0f, 20.0f, 20.0f ) );

	SpawnCollison = CreateDefaultSubobject<UBoxComponent>( TEXT( "Spawn Collision" ) );
	SpawnCollison->AttachToComponent( FloorMesh, FAttachmentTransformRules::KeepRelativeTransform );
	SpawnCollison->SetCollisionEnabled( ECollisionEnabled::QueryOnly );
	SpawnCollison->bGenerateOverlapEvents = true;

	UpgradeSpawnZone = CreateDefaultSubobject<UBoxComponent>( TEXT( "Upgade Spawn Zone" ) );
	UpgradeSpawnZone->AttachToComponent( Root, FAttachmentTransformRules::KeepRelativeTransform );
	UpgradeSpawnZone->SetCollisionEnabled( ECollisionEnabled::NoCollision );
	UpgradeSpawnZone->bGenerateOverlapEvents = false;
	UpgradeSpawnZone->SetWorldScale3D( FVector( 0.0f, 0.0f, 0.0f ) );
}

void ABaseFloorPiece::Cleanup()
{
	for( auto multi_piece : MultiConnections )
	{
		if( multi_piece.ConnectedSpawnPiece )
		{
			multi_piece.ConnectedSpawnPiece->Cleanup();
			multi_piece.ConnectedSpawnPiece->Destroy();
		}
	}

	if( UpgradeActor )
		UpgradeActor->Destroy();
}

void ABaseFloorPiece::BeginPlay()
{
	Super::BeginPlay();

	// Random chance to spawn an upgrade
	const auto* GameData = UCubeSingletonDataLibrary::GetGameData();

	if( GameData->DefaultUpgradeBPClass )
	{
		if( UpgradeSpawnZone->GetComponentScale() != FVector( 0.0f, 0.0f, 0.0f ) )
		{
			// Chance to spawn an upgrade
			if( GameData->UpgradeSpawnChancePerPiecePercent > FMath::RandRange( 0, 99 ) )
			{
				int32 safety = 0;

				while( safety++ < 20 && !UpgradeActor )
				{
					// Location
					const auto Origin = UpgradeSpawnZone->GetComponentLocation();
					const auto BoxExtent = UpgradeSpawnZone->Bounds.BoxExtent;
					auto RandPos = UKismetMathLibrary::RandomPointInBoundingBox( Origin, FVector( BoxExtent.X - 60.0f, BoxExtent.Y - 60.0f, 0.0f ) );
					FRotator Rotation( 0.0f, 0.0f, 0.0f );

					// Find correct height to hover at
					FCollisionQueryParams trace_params = FCollisionQueryParams( FName( TEXT( "Spawn_Trace" ) ), true, nullptr );
					FHitResult trace_hit( ForceInit );

					auto start = FVector( RandPos.X, RandPos.Y, FloorMesh->GetComponentLocation().Z + ObstacleSpawnTraceHeight );
					auto end = FVector( RandPos.X, RandPos.Y, FloorMesh->GetComponentLocation().Z - ObstacleSpawnTraceHeight );

					if ( !ActorLineTraceSingle( trace_hit, start, end, ECC_Visibility, trace_params ) )
					{
						UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::BeginPlay | UpgradeBPClass failed to adjust hover height due to line trace returning NULL", LogDisplayType::Warn );
					}
					else
					{
						RandPos.Z += trace_hit.ImpactPoint.Z;
						Rotation = FRotationMatrix::MakeFromX( trace_hit.ImpactNormal ).ToQuat().Rotator();
					}

					// Spawn params
					FActorSpawnParameters SpawnParams;
					SpawnParams.bAllowDuringConstructionScript = false;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
					SpawnParams.bDeferConstruction = false;

					UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::BeginPlay | Tying to spawn upgrade: " + GameData->DefaultUpgradeBPClass->GetPathName() +
						", Pos: " + FString::FromInt( RandPos.X ) + ", " + FString::FromInt( RandPos.Y ) );
					UpgradeActor = GetWorld()->SpawnActor( GameData->DefaultUpgradeBPClass, &RandPos, &Rotation, SpawnParams );
				}

				if( safety >= 20 )
					UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::BeginPlay | UpgradeBPClass failed to spawn", LogDisplayType::Warn );
			}
		}
	}
	else UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::BeginPlay | UpgradeBPClass UClass not valid ", LogDisplayType::Warn );
}

void ABaseFloorPiece::OnConstruction( const FTransform& Transform )
{
	Super::OnConstruction( Transform );

	ConstructionScriptRun = false;

	auto scale = UpgradeSpawnZone->GetComponentScale();
	scale.Z = 0.0f;
	UpgradeSpawnZone->SetWorldScale3D( scale );

	TArray< USceneComponent* > children;
	GetRootComponent()->GetChildrenComponents( true, children );

	// Destroys obstacles that don't match the current variation (not permanently) 
	for( auto* component : children )
	{
		auto* child_actor = Cast< UChildActorComponent >( component );

		if( IsValid( child_actor ) )
		{
			auto* obstacle = Cast< ABaseObstacle >( child_actor->GetChildActor() );

			if( IsValid( obstacle ) && obstacle->Variation != 0 && obstacle->Variation != Variation )
				obstacle->Destroy();
		}

		auto* obstacle = Cast< UBaseObstacleComponent >( component );

		if( IsValid( obstacle ) && obstacle->Variations.Num() )
			if( obstacle->Variations.Num() && obstacle->Variations[0] != 0 && !obstacle->Variations.Contains( Variation ) )
				obstacle->DestroyComponent();
	}
}

void ABaseFloorPiece::DestroyObstacles()
{
	for( auto& instance : InstancedObstacleData )
	{
		if( instance.Value.InstancedStaticMesh && instance.Value.InstancedStaticMesh->IsValidLowLevel() )
		{
			instance.Value.InstancedStaticMesh->UnregisterComponent();
			instance.Value.InstancedStaticMesh->DestroyComponent();
			instance.Value.Data.Reset();
		}
	}

	InstancedObstacleData.Empty();

	for( auto obstacle : SpawnedChildObstacles )
	{
		if( obstacle.Component && obstacle.Component->IsValidLowLevel() )
		{
			obstacle.Component->UnregisterComponent();
			obstacle.Component->DestroyComponent();
		}
	}

	SpawnedChildObstacles.Empty();

	ConstructionScriptRun = true;
}

void ABaseFloorPiece::FloorPieceBeginPlay()
{
	OnFloorPieceBeginPlay();
}

void ABaseFloorPiece::AddMultiConnection( UArrowComponent* Connection, UBoxComponent* Collider )
{
	if( !MultiConnections.Num() )
		MultiConnections.Add( FSplitConnection( ConnectionPoint, SpawnCollison ) );

	MultiConnections.Add( FSplitConnection( Connection, Collider ) );
}

FVector ABaseFloorPiece::Bezier( float Interval, TArray< FVector > ControlPoints )
{
	if( ControlPoints.Num() == 3 )
		return BezierQuadratic( Interval, ControlPoints[ 0 ], ControlPoints[ 1 ], ControlPoints[ 2 ] );

	if( ControlPoints.Num() == 4 )
		return BezierCubic( Interval, ControlPoints[ 0 ], ControlPoints[ 1 ], ControlPoints[ 2 ], ControlPoints[ 3 ] );

	FVector Result( 0.0f, 0.0f, 0.0f );
	int32 n = ControlPoints.Num() - 1;

	for( int32 k = 0; k < ControlPoints.Num(); ++k )
	{
		Result += ControlPoints[ k ] * Binomial( n, k ) * FMath::Pow( 1.0f - Interval, n - k ) * FMath::Pow( Interval, k );
	}

	return Result;
}

FVector ABaseFloorPiece::BezierQuadratic( float Interval, FVector Start, FVector Corner, FVector End )
{
	const auto t2 = Interval * Interval;
	const auto mt = 1.0f - Interval;
	const auto mt2 = mt * mt;
	return Start * mt2 + Corner * 2.0f * mt * Interval + End * t2;
}

FVector ABaseFloorPiece::BezierCubic( float Interval, FVector Start, FVector CornerA, FVector CornerB, FVector End )
{
	const auto t2 = Interval * Interval;
	const auto t3 = t2 * Interval;
	const auto mt = 1.0f - Interval;
	const auto mt2 = mt * mt;
	const auto mt3 = mt2 * mt;
	return Start * mt3 + 3.0f * CornerA * mt2 * Interval + 3.0f * CornerB * mt * t2 + End * t3;
}

void ABaseFloorPiece::SpawnObstacles( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, int32 SpawnVariation, bool SpawnEvenly /*= false*/, UClass* Class /*= nullptr*/, int32 BezierSteps /*= 150*/ )
{
	SpawnObstaclesWithMask( ControlPoints, Count, SpawnStyle, TArray< int32 >(), SpawnVariation, SpawnEvenly, Class, BezierSteps );
}

void ABaseFloorPiece::SpawnObstaclesMultiVariations( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > SpawnVariations, bool SpawnEvenly /*= false*/, UClass* Class /*= nullptr*/, int32 BezierSteps /*= 150*/ )
{
	SpawnObstaclesWithMaskMultiVariations( ControlPoints, Count, SpawnStyle, TArray< int32 >(), SpawnVariations, SpawnEvenly, Class, BezierSteps );
}

void ABaseFloorPiece::SpawnObstaclesWithMask( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, int32 SpawnVariation, bool SpawnEvenly /*= false*/, UClass* Class/*= nullptr*/, int32 BezierSteps /*= 150*/ )
{
	SpawnObstaclesWithMaskMultiVariations( ControlPoints, Count, SpawnStyle, Mask, { SpawnVariation }, SpawnEvenly, Class, BezierSteps );
}

void ABaseFloorPiece::SpawnObstaclesWithMaskMultiVariations( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, bool SpawnEvenly /*= false*/, UClass* Class /*= nullptr*/, int32 BezierSteps /*= 150*/ )
{
	if( SpawnEvenly )
		SpawnObstaclesEvenlyWithMaskInternal( ControlPoints, Count, SpawnStyle, Mask, SpawnVariations, Class, BezierSteps );
	else
		SpawnObstaclesWithMaskInternal( ControlPoints, Count, SpawnStyle, Mask, SpawnVariations, Class );
}

void ABaseFloorPiece::SpawnObstaclesWithMaskInternal( TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, UClass* Class /*= nullptr*/ )
{
	if( SpawnVariations.Num() > 0 && SpawnVariations[0] != 0 && !SpawnVariations.Contains( Variation ) )
		return;

	if( Class == nullptr )
		Class = FindObstacleClass();

	if( !Class->IsChildOf< UPrimitiveComponent >() )
	{
		UCubeSingletonDataLibrary::CustomLog( "Spawning obstacle failed with invalid class type: " + ( Class ? Class->GetName() : "INVALID CLASS" ), LogDisplayType::Error );
		Class = FindObstacleClass();
	}

	if( !ConstructionScriptRun )
		DestroyObstacles();

	if( SpawnVariations.Num() )
	{
		SpawnVariations.Sort();
		MaxVariationClassic = FMath::Max( MaxVariationClassic, SpawnVariations.Last() );
		MaxVariationAdvanced = FMath::Max( MaxVariationAdvanced, SpawnVariations.Last() );
	}

	Count += 2;
	bool StatePlacing = true;
	int32 StateCounter = 0;
	int32 MaskCounter = ( Mask.Num() > 0 ? Mask[ 0 ] : -1 );

	for( int32 i = 1; i < Count; ++i )
	{
		if( MaskCounter-- == 0 )
		{
			if( ++StateCounter < Mask.Num() )
			{
				MaskCounter = Mask[ StateCounter ];
				StatePlacing = !StatePlacing;
			}
			else StatePlacing = true;
		}

		if( StatePlacing )
		{
			float Interval = ( float )i / ( float )Count;
			FTransform Transform( Bezier( Interval, ControlPoints ) );

			if( SpawnStyle == ESpawnObstaclesType::ESOAT_FOLLOW_ROTATION )
			{
				Interval += 0.01f;
				auto TargetDirection = Bezier( Interval, ControlPoints ) - Transform.GetLocation();
				TargetDirection.Normalize();
				Transform.SetRotation( FRotationMatrix::MakeFromX( TargetDirection ).ToQuat() );
			}
			else if( SpawnStyle == ESpawnObstaclesType::ESOAT_ATTACH )
			{
				// Position line trace
				FCollisionQueryParams trace_params = FCollisionQueryParams( FName( TEXT( "Spawn_Trace" ) ), true, nullptr );
				FHitResult trace_hit( ForceInit );

				const auto Position = Transform.GetLocation();
				const auto start = FVector( Position.X, Position.Y, FloorMesh->GetComponentLocation().Z + ObstacleSpawnTraceHeight );
				const auto end = FVector( Position.X, Position.Y, FloorMesh->GetComponentLocation().Z - ObstacleSpawnTraceHeight );
				
				if( ActorLineTraceSingle( trace_hit, start, end, ECC_Visibility, trace_params ) )
				{
					Transform.SetLocation( trace_hit.ImpactPoint + FVector( 0.0f, 0.0f, 75.0f ) );
					Transform.SetRotation( FRotationMatrix::MakeFromX( trace_hit.ImpactNormal ).ToQuat() );
				}
				//else UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::SpawnObstaclesWithMaskInternal | Spawning failed with ESOAT_ATTACH style as line trace returned NULL", Warn );
			}

			SpawnObstacleMultiVariations( Class, Transform, SpawnVariations );
		}
	}
}

void ABaseFloorPiece::SpawnObstaclesEvenlyWithMaskInternal( TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, UClass* Class /*= nullptr*/, int32 BezierSteps /*= 150*/ )
{
	if( SpawnVariations.Num() > 0 && SpawnVariations[0] != 0 && !SpawnVariations.Contains( Variation ) )
		return;

	if( Class == nullptr )
		Class = FindObstacleClass();

	if( !Class->IsChildOf< UPrimitiveComponent >() )
	{
		UCubeSingletonDataLibrary::CustomLog( "Spawning obstacle failed with invalid class type: " + ( Class ? Class->GetName() : "INVALID CLASS" ), LogDisplayType::Error );
		Class = FindObstacleClass();
	}

	if( !ConstructionScriptRun )
		DestroyObstacles();

	if( SpawnVariations.Num() )
	{
		SpawnVariations.Sort();
		MaxVariationClassic = FMath::Max( MaxVariationClassic, SpawnVariations.Last() );
		MaxVariationAdvanced = FMath::Max( MaxVariationAdvanced, SpawnVariations.Last() );
	}

	//const auto Steps = 150;// Count * 2; // Look into a good value for this
	const float BezierLength = CalculateBezierCurveLength( ControlPoints, BezierSteps );
	const auto DistanceThreshold = BezierLength / ( Count + 1 );
	auto DistanceCounter = 0.0f;
	auto PreviousPosition = Bezier( 0.0f, ControlPoints );

	bool StatePlacing = true;
	int32 StateCounter = 0;
	int32 MaskCounter = ( Mask.Num() > 0 ? Mask[ 0 ] : -1 );

	for( int32 i = 1; i <= BezierSteps; ++i )
	{
		const float Interval = ( float )i / ( float )BezierSteps;
		const auto NewPosition = Bezier( Interval, ControlPoints );
		DistanceCounter += ( NewPosition - PreviousPosition ).Size();

		if( DistanceCounter >= DistanceThreshold )
		{
			DistanceCounter = 0.0f;

			--MaskCounter;
			if( MaskCounter == 0 )
			{
				++StateCounter;
				if( StateCounter < Mask.Num() )
				{
					MaskCounter = Mask[ StateCounter ];
					StatePlacing = !StatePlacing;
				}
				else StatePlacing = true;
			}

			if( StatePlacing )
			{
				FTransform Transform( NewPosition );

				if( SpawnStyle == ESpawnObstaclesType::ESOAT_FOLLOW_ROTATION )
				{
					auto TargetDirection = NewPosition - PreviousPosition;
					TargetDirection.Normalize();
					Transform.SetRotation( FRotationMatrix::MakeFromX( TargetDirection ).ToQuat() );
				}
				else if( SpawnStyle == ESpawnObstaclesType::ESOAT_ATTACH )
				{
					// Position line trace
					FCollisionQueryParams trace_params = FCollisionQueryParams( FName( TEXT( "Spawn_Trace" ) ), true, nullptr );
					FHitResult trace_hit( ForceInit );

					const auto start = FVector( NewPosition.X, NewPosition.Y, FloorMesh->GetComponentLocation().Z + ObstacleSpawnTraceHeight );
					const auto end = FVector( NewPosition.X, NewPosition.Y, FloorMesh->GetComponentLocation().Z - ObstacleSpawnTraceHeight );

					if( ActorLineTraceSingle( trace_hit, start, end, ECC_Visibility, trace_params ) )
					{
						Transform.SetLocation( trace_hit.ImpactPoint + FVector( 0.0f, 0.0f, 75.0f ) );
						Transform.SetRotation( FRotationMatrix::MakeFromX( trace_hit.ImpactNormal ).ToQuat() );
					}
					//else UCubeSingletonDataLibrary::CustomLog( "ABaseFloorPiece::SpawnObstaclesEvenlyWithMaskInternal | Spawning failed with ESOAT_ATTACH style as line trace returned NULL", Warn );
				}

				SpawnObstacleMultiVariations( Class, Transform, SpawnVariations );
			}
		}

		PreviousPosition = NewPosition;
	}
}

void ABaseFloorPiece::SpawnObstacle( UClass* Class, FTransform Transform, int32 SpawnVariation )
{
	SpawnObstacleMultiVariations( Class, Transform, { SpawnVariation } );
}

void ABaseFloorPiece::SpawnObstacleMultiVariations( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations )
{
	if( InstancedObstacleSpawningEnabled )
	{
		SpawnInstancedObstacleInternal( Class, Transform, SpawnVariations, EObjectFlags::RF_NoFlags );
	}
	else
	{
		if( const auto NewComponent = SpawnObstacleInternal( Class, Transform, SpawnVariations, EObjectFlags::RF_NoFlags ) )
		{
			FChildObstacle NewObstacle( NewComponent, SpawnVariations );
			SpawnedChildObstacles.Add( NewObstacle );
		}
	}
}

void ABaseFloorPiece::SpawnInstancedObstacleInternal( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations, EObjectFlags Flags )
{
	auto* Mesh = Cast< UStaticMeshComponent >( Class->GetDefaultObject() )->GetStaticMesh();

	auto* Result = InstancedObstacleData.Find( Mesh );

	if( Result )
	{
		Result->InstancedStaticMesh->AddInstance( Transform );
		Result->Data.Add( FInstancedObstacleData( SpawnVariations ) );
	}
	else
	{
		UInstancedStaticMeshComponent* ISMComp = NewObject< UInstancedStaticMeshComponent >( this );

		if( ISMComp )
		{
			ISMComp->AttachToComponent( RootComponent, FAttachmentTransformRules::KeepRelativeTransform );
			ISMComp->RegisterComponent();
			ISMComp->SetWorldTransform( FTransform() );
			ISMComp->SetStaticMesh( Mesh );
			ISMComp->AddInstance( Transform );
			InstancedObstacleData.Add( Mesh, FInstancedObstacleDataContainer( ISMComp, FInstancedObstacleData( SpawnVariations ) ) );
		}
		else UCubeSingletonDataLibrary::CustomLog( "Spawning UInstancedStaticMeshComponent failed", LogDisplayType::Error );
	}
}

UStaticMeshComponent* ABaseFloorPiece::SpawnObstacleInternal( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations, EObjectFlags Flags )
{
	// Create obstacle (component)
	auto* NewComponent = NewObject<UStaticMeshComponent>( this, Class, NAME_None, Flags );

	if( NewComponent )
	{
		NewComponent->AttachToComponent( RootComponent, FAttachmentTransformRules::KeepRelativeTransform );
		NewComponent->RegisterComponent();
		NewComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		NewComponent->SetWorldTransform( Transform );
		return NewComponent;
	}
	else UCubeSingletonDataLibrary::CustomLog( "Spawning obstacle failed with valid class type: " + Class->GetName(), LogDisplayType::Error );

	return nullptr;
}

FVector ABaseFloorPiece::CalculateCornerPosition( FVector Start, FVector End, FVector StartForward )
{
	const auto DirectionFromEnd = End - Start;
	const auto DistFromStart = FVector::DotProduct( DirectionFromEnd, StartForward );
	return Start + StartForward * DistFromStart;
}

TArray< FVector > ABaseFloorPiece::FindTurnControlPoints( USceneComponent* Start, USceneComponent* End )
{
	const auto StartPos = Start->GetComponentLocation();
	const auto EndPos = End->GetComponentLocation();

	TArray< FVector > output;
	output.Add( StartPos );
	output.Add( CalculateCornerPosition( StartPos, EndPos, Start->GetForwardVector() ) );
	output.Add( EndPos );

	return output;
}

float ABaseFloorPiece::CalculateBezierCurveLength( TArray< FVector > ControlPoints, const int32 StepCount )
{
	float Length = 0.0f;
	float Step = 0.0f;

	for( int32 i = 0; i < StepCount - 1; ++i )
	{
		const auto Point0 = Bezier( Step, ControlPoints );
		Step += ( 1.0f / StepCount );
		const auto Point1 = Bezier( Step, ControlPoints );
		Length += ( Point1 - Point0 ).Size();
	}

	return Length;
}

int32 ABaseFloorPiece::Binomial( int32 n, int32 k )
{
	if( BinomialLookUpTable.Num() == 0 )
		InitialiseLookUpTable();

	while( n >= BinomialLookUpTable.Num() )
	{
		TArray< int32 > NewRow;
		NewRow.Add( 1 );
		int32 length = BinomialLookUpTable.Num() - 1;

		for( int32 i = 1; i < length; i++ )
		{
			NewRow.Add( BinomialLookUpTable[ length ][ i - 1 ] + BinomialLookUpTable[ length ][ i ] );
		}

		NewRow.Add( 1 );
		BinomialLookUpTable.Add( NewRow );
	}

	return BinomialLookUpTable[ n ][ k ];
}

void ABaseFloorPiece::InitialiseLookUpTable()
{
	std::vector< std::vector< int32 > > values
	{
		{ 1 },
		{ 1, 1 },
		{ 1, 2, 1 },
		{ 1, 3, 3, 1 },
		{ 1, 4, 6, 4, 1 },
		{ 1, 5, 10, 10, 5, 1 },
		{ 1, 6, 15, 20, 15, 6, 1 }
	};

	BinomialLookUpTable.AddDefaulted( values.size() );

	for( size_t i = 0; i < values.size(); ++i )
	{
		BinomialLookUpTable[ i ].AddDefaulted( values[ i ].size() );

		for( size_t j = 0; j < values[ i ].size(); ++j )
		{
			BinomialLookUpTable[ i ][ j ] = values[ i ][ j ];
		}
	}
}

bool ABaseFloorPiece::IsReadyToBePlaced()
{
	return CoolDownCounter == 0;
}

void ABaseFloorPiece::NewFloorPieceSpawned( ABaseFloorPiece* NewPiece )
{
	// Reduce cooldown
	CoolDownCounter = FMath::Max( CoolDownCounter--, 0 );

	// Assuming you can't have a piece that is both a right and left turn!
	bool LeftTurn = this->Tags.Find( TEXT( "LEFT TURN" ) ) != INDEX_NONE || NewPiece->Tags.Find( TEXT( "LEFT TURN" ) ) != INDEX_NONE;
	bool RightTurn = this->Tags.Find( TEXT( "RIGHT TURN" ) ) != INDEX_NONE || NewPiece->Tags.Find( TEXT( "RIGHT TURN" ) ) != INDEX_NONE;

	if( LeftTurn && RightTurn )
		CoolDownCounter = 0;
}

UClass* ABaseFloorPiece::FindObstacleClass()
{
	const auto* GameData = UCubeSingletonDataLibrary::GetGameData();
	UClass* Class = GameData->ClassicCubeObstacleBPClass;

	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );

	if( IsValid( GameInstance ) && !GameInstance->ClassicPlayerMode )
		Class = GameData->AdvancedCubeObstacleBPClass;

	return Class;
}