// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "BaseFloorPiece.generated.h"

UENUM( BlueprintType )
enum class EPieceFamily : uint8
{
	EPF_RANDOMISED UMETA( DisplayName = "Randomised" ),
	EPF_CUBE UMETA( DisplayName = "Cube" ),
	EPF_WALL UMETA( DisplayName = "Wall" ),
	EPF_SKY UMETA( DisplayName = "Sky" ),
	EPF_SLOPE UMETA( DisplayName = "Slope" ),
	EPF_PLATFORM UMETA( DisplayName = "Platform" ),
	EPF_NONE UMETA( DisplayName = "None" ),
};

UENUM( BlueprintType )
enum class ESpawnObstaclesType : uint8
{
	ESOAT_DEFAULT UMETA( DisplayName = "Default" ),
	ESOAT_FOLLOW_ROTATION UMETA( DisplayName = "Follow Rotation" ),
	ESOAT_ATTACH UMETA( DisplayName = "Attach To Floor Piece" ),
};

USTRUCT( BlueprintType )
struct FChildObstacle
{
	GENERATED_USTRUCT_BODY()

public:
	FChildObstacle() {}
	FChildObstacle( UStaticMeshComponent* Comp, TArray< int32 > _Variations ) : Component( Comp ), Variations( _Variations ) {}

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) UStaticMeshComponent* Component;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< int32 > Variations;
};

USTRUCT( BlueprintType )
struct FSplitConnection
{
	GENERATED_USTRUCT_BODY()

public:
	FSplitConnection() {}
	FSplitConnection( UArrowComponent* Con, UBoxComponent* Col ) : ConnectionPoint( Con ), Collider( Col ) {}

	class UArrowComponent* ConnectionPoint = nullptr;
	class UBoxComponent* Collider = nullptr;
	class ABaseFloorPiece* ConnectedSpawnPiece = nullptr;
};

USTRUCT( BlueprintType )
struct FInstancedObstacleData
{
	GENERATED_USTRUCT_BODY()

public:
	FInstancedObstacleData() {}
	FInstancedObstacleData( TArray< int32 >  _Variations ) : Variations( _Variations ) {}

	// Members
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 WaypointIndex;
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< FVector > WaypointPositions;
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float MovementSpeed;
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool RotateTowardsTarget;
	//UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) EMovementStyle MovementStyle;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< int32 >  Variations;
};

USTRUCT( BlueprintType )
struct FInstancedObstacleDataContainer
{
	GENERATED_USTRUCT_BODY()

public:
	FInstancedObstacleDataContainer() {}
	FInstancedObstacleDataContainer( UInstancedStaticMeshComponent* _InstancedStaticMesh, FInstancedObstacleData _Data ) : InstancedStaticMesh( _InstancedStaticMesh ), Data()
	{
		Data.Add( _Data );
	}

	// Members
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) UInstancedStaticMeshComponent* InstancedStaticMesh;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< FInstancedObstacleData > Data;
};

UCLASS()
class CUBERUNNER_API ABaseFloorPiece : public AActor
{
	GENERATED_BODY()

		// Functions
public:
	ABaseFloorPiece( const FObjectInitializer& ObjectInitializer );

	virtual void Cleanup();
	virtual void BeginPlay() override;
	virtual void OnConstruction( const FTransform& Transform ) override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Core" )
	void OnFloorPieceBeginPlay();

	virtual void FloorPieceBeginPlay();

	virtual bool IsReadyToBePlaced();
	void NewFloorPieceSpawned( ABaseFloorPiece* NewPiece );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	UClass* FindObstacleClass();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void AddMultiConnection( UArrowComponent* ConnectionPoint, UBoxComponent* Collider );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstacles( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, int32 SpawnVariation, bool SpawnEvenly = false, UClass* Class = nullptr, int32 BezierSteps = 150 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstaclesMultiVariations( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > SpawnVariations, bool SpawnEvenly = false, UClass* Class = nullptr, int32 BezierSteps = 150 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstaclesWithMask( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, int32 SpawnVariation, bool SpawnEvenly = false, UClass* Class = nullptr, int32 BezierSteps = 150 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstaclesWithMaskMultiVariations( UPARAM( ref )TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, bool SpawnEvenly = false, UClass* Class = nullptr, int32 BezierSteps = 150 );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstacle( UClass* Class, FTransform Transform, int32 SpawnVariation );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SpawnObstacleMultiVariations( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	FVector CalculateCornerPosition( FVector Start, FVector End, FVector StartForward );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	TArray< FVector > FindTurnControlPoints( USceneComponent* Start, USceneComponent* End );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	FVector Bezier( float Interval, TArray< FVector > ControlPoints );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	FVector BezierQuadratic( float Interval, FVector Start, FVector Corner, FVector End );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	FVector BezierCubic( float Interval, FVector Start, FVector CornerA, FVector CornerB, FVector End );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	float CalculateBezierCurveLength( TArray< FVector > ControlPoints, const int32 StepCount );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	int32 Binomial( int32 n, int32 k );

	static void InitialiseLookUpTable();

protected:
	void DestroyObstacles();

	UStaticMeshComponent* SpawnObstacleInternal( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations, EObjectFlags Flags  );
	void SpawnObstaclesWithMaskInternal( TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, UClass* Class = nullptr );
	void SpawnObstaclesEvenlyWithMaskInternal( TArray< FVector >& ControlPoints, int32 Count, ESpawnObstaclesType SpawnStyle, TArray< int32 > Mask, TArray< int32 > SpawnVariations, UClass* Class = nullptr, int32 BezierSteps = 150 );
	void SpawnInstancedObstacleInternal( UClass* Class, FTransform Transform, TArray< int32 > SpawnVariations, EObjectFlags Flags );

	// Members
public:
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class USceneComponent* Root;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UArrowComponent* RootDir;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UBoxComponent* SpawnCollison;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UArrowComponent* ConnectionPoint;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UStaticMeshComponent* FloorMesh;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UBoxComponent* UpgradeSpawnZone;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) EPieceFamily PieceFamily;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 CoolDownLength;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool TransitionPiece;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 Variation;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 MaxVariationClassic;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) int32 MaxVariationAdvanced;
	AActor* UpgradeActor;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) TArray< FSplitConnection > MultiConnections;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) bool EndLevelPiece;

	UPROPERTY( Transient, BlueprintReadOnly, Category = Data ) TArray< FChildObstacle > SpawnedChildObstacles;
	UPROPERTY( Transient, BlueprintReadOnly, Category = Data ) TMap< UStaticMesh*, FInstancedObstacleDataContainer > InstancedObstacleData;

	int32 CoolDownCounter;
	bool ConstructionScriptRun;
	bool HasTriggered;

private:
	static TArray< TArray < int32 > > BinomialLookUpTable;
};
