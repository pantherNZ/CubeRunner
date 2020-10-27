// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include <unordered_map>

#include "GameFramework/GameMode.h"
#include "BaseFloorPiece.h"
#include "BasePlayerPawn.h"
#include "CubeRunnerGameMode.generated.h"

USTRUCT( BlueprintType )
struct FFloorPieceType
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) UClass* FloorPieceBPClass;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) int32 Difficulty;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) float Probability;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) EPieceFamily Family;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) int32 Connections;

	bool operator < ( const FFloorPieceType& piece ) const
	{
		return Difficulty < piece.Difficulty;
	}
};

USTRUCT( BlueprintType )
struct FFloorPieceFamily
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) EPieceFamily Family;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) UClass* StartTransitionPiece;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) UClass* EndTransitionPiece;
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) bool PrivateFamily;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, meta = ( EditCondition = "PrivateFamily" ) ) int32 PrivateFamilyLengthMin;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, meta = ( EditCondition = "PrivateFamily" ) ) int32 PrivateFamilyLengthMax;
};

struct FSpawnQueueItem
{
	~FSpawnQueueItem()
	{
		if( PropagateDeletionToChildren )
		{
			for( int32 i = 0; i < NextQueueItems.Num(); ++i )
			{
				NextQueueItems[ i ]->PropagateDeletionToChildren = true;
				delete NextQueueItems[ i ];
			}
		}
	}

	bool PropagateDeletionToChildren = false;
	UClass* PieceClass = nullptr;
	int32 Variation = -1;
	int32 QueueIndex = 0;
	TArray< FSpawnQueueItem* > NextQueueItems;
};

UENUM( BlueprintType )
enum class EGameEndState : uint8
{
	EGES_FAIL UMETA( DisplayName = "Hit Obstacle" ),
	EGES_SUCCESS UMETA( DisplayName = "Level Complete" )
};

UENUM( BlueprintType )
enum class ERegistryType : uint8
{
	ERT_SHARED UMETA( DisplayName = "Shared" ),
	ERT_CLASSIC UMETA( DisplayName = "Classic" ),
	ERT_ADVANCED UMETA( DisplayName = "Advanced" )
};

UCLASS()
class CUBERUNNER_API ACubeRunnerGameMode : public AGameMode
{
	GENERATED_BODY()

	// Functions
public:
	ACubeRunnerGameMode( const FObjectInitializer& ObjectInitializer );
	~ACubeRunnerGameMode() { }

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;

	void SpawnExtraConnections( ABaseFloorPiece* BaseMultiPiece );
	void MultiPieceCollision( ABaseFloorPiece* BaseMultiPiece, const int32 index );

	UFUNCTION( BlueprintImplementableEvent, Category = "Setup" )
	void RegisterFloorPieceFamilies();

	UFUNCTION( BlueprintImplementableEvent, Category = "Setup" )
	void RegisterFloorPieceClasses();

	UFUNCTION( BlueprintImplementableEvent, Category = "Setup" )
	void LoadClassicLevel( int32 Level );

	UFUNCTION( BlueprintImplementableEvent, Category = "Setup" )
	void LoadAdvancedLevel( int32 Level );

	UFUNCTION( BlueprintImplementableEvent, Category = "Events" )
	void OnUpgradeAdded( ABaseUpgrade* Upgrade );

	UFUNCTION( BlueprintImplementableEvent, Category = "Events" )
	void ForceLevelUIReload();

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void SpawnFloorPiece( const FTransform& Transform, UClass* PieceOverride = nullptr, int32 VariationOverride = 0 );

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void RemoveFloorPiece();

	UFUNCTION( BlueprintCallable, BlueprintNativeEvent, Category = "Events" )
	void GameEnd( EGameEndState Reason );

	UFUNCTION( BlueprintCallable, Category = "Events" )	
	void LoadMainMenu();

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void TempRestartGame();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePiece( UClass* Class );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceWithVariation( UClass* Class, int32 Variation );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceWithRandomVariation( UClass* Class );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceMulti( UClass* Class, int32 Count );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceMultiWithVariation( UClass* Class, int32 Count, int32 Variation );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceMultiWithRandomVariation( UClass* Class, int32 Count );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceSplit( UClass* Class );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceSplitWithVariation( UClass* Class, int32 Variation );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void QueuePieceSplitWithRandomVariation( UClass* Class );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void EndSplitPieceQueue();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SetLevelOptions( bool PreSpawnFloorPieces, float PlayerStartSpeed, float PlayerAcceleration, bool SpawnTransitions, FVector StartLocation, FTransform PlayerStartTransform, bool SpawnAfterFinish = true );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void SetLevelOptionsWithPreSpawn( int32 FloorPiecesToPreSpawn, float PlayerStartSpeed, float PlayerAcceleration, bool SpawnTransitions, FVector StartLocation, FTransform PlayerStartTransform, bool SpawnAfterFinish = true );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void RegisterFloorPieceBPClass( UClass* FloorPieceClass, int32 Difficulty, float Probability, EPieceFamily Family, int32 Connections = 1, ERegistryType Type = ERegistryType::ERT_SHARED );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void RegisterFamily( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family, ERegistryType Type = ERegistryType::ERT_SHARED );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void RegisterFamilyWithLength( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family, 
		int32 PrivateFamilyLengthMin, int32 PrivateFamilyLengthMax, ERegistryType Type = ERegistryType::ERT_SHARED );
	
	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void RegisterFamilyWithLength2( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family,
		int32 PrivateFamilyLengthMin, int32 PrivateFamilyLengthMax, int32 Difficulty, float Probability, ERegistryType Type = ERegistryType::ERT_SHARED );

private:
	bool CheckValidGameType( ERegistryType Type ) const;
	void FindFloorPieceToSpawn( const bool IgnoreSplitPieces = false );
	FVector LocationRounded( const FVector& Loc );
	void DestroyPawn();
	ABaseFloorPiece* SpawnFloorPieceInternal( UClass* PieceClass, int32 PieceVariation, bool TransitionPiece, const FTransform& Transform, const bool AddToArray );

	// Members
public:
	UPROPERTY( BlueprintReadWrite, Category = "Data" ) ABasePlayerPawn* PlayerRef;
	UPROPERTY( BlueprintReadWrite, Category = "Data" ) bool EndlessMode;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) TArray< FFloorPieceType > FloorPieceBPClasses;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) UClass* FloorPieceOverride;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 RemovalDelay;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) float GameProgress;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) float GameProgressPerMinute;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) UClass* ClassicPawnClass;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) UClass* AdvancedPawnClass;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) float LevelFogOpacity;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 LevelRandomisedFloorPieceDensity;

	// Backend data	
	TArray< ABaseFloorPiece* > FloorPieceArray;
	float DistanceMoved;
	bool UpdateNewFloorPiecePosition;	
	int32 RepeatCount;
	int32 PreSpawnedPieces;
	bool ClassicMode;

	EPieceFamily PrivateFamily;
	int32 PrivateLengthRemaining;

	FSpawnQueueItem* SpawnQueueRoot;
	FSpawnQueueItem* SpawnQueueCurrent;
	TArray< FSpawnQueueItem* > SpawnQueueSplits;

	// Level settings
	bool LevelOptionsSet;
	bool LevelPreSpawningEnabled;
	int32 LevelPreSpawningCount;
	bool LevelSpawnTransitions;
	float LevelPlayerStartSpeed;
	float LevelPlayerAcceleration;
	FVector LevelStartLocation;
	bool LevelSpawnAfterFinish;
	FTransform LevelPlayerStartTransform;

	// Timer to destroy pawn after game ends
	FTimerHandle pawn_destroy_handle;

	// Storage for data specific to a child blueprint piece
	TMap< UClass*, uint32 > PieceCooldownData;

	// Storage for family data 
	TMap< EPieceFamily, FFloorPieceFamily > PieceFamilyData;
};
