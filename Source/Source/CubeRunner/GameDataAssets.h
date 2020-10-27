// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseFloorPiece.h"
#include "BaseUpgrade.h"
#include "BaseObstacleComponent.h"
#include "GameDataAssets.generated.h"

USTRUCT( BlueprintType )
struct FLevelInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FLevelInfo() { }
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) FString Name;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) UTexture2D* Image = nullptr;
};

USTRUCT( BlueprintType )
struct FUpgradeInfo
{
	GENERATED_USTRUCT_BODY()

public:
	FUpgradeInfo() : SpawnChance( 100 ), IsNegative( false ) { }
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) FString Name;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float SpawnChance;
	UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = Data ) bool IsNegative;
};

UCLASS( BlueprintType )
class CUBERUNNER_API UGameDataAssets : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameDataAssets( const FObjectInitializer& ObjectInitializer );

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TSubclassOf< ABaseFloorPiece > RandomisedFloorPieceBPClass;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TSubclassOf< ABaseUpgrade > DefaultUpgradeBPClass;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TSubclassOf< UBaseObstacleComponent > ClassicCubeObstacleBPClass;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TSubclassOf< UBaseObstacleComponent > AdvancedCubeObstacleBPClass;

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) class UParticleSystem* UpgradeParticle;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) class UParticleSystem* ExplosionParticle;

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) int32 LevelsPerDifficulty;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) int32 ClassicDifficultyLevels;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) int32 AdvancedDifficultyLevels;

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TArray< FStringAssetReference > AssetsToLoad;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TArray< FString > FoldersToLoad;

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) int32 UpgradeSpawnChancePerPiecePercent;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) int32 UpgradeIsNegativeEffectChancePercent;

	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TArray< FLevelInfo > ClassicLevelInformation;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TArray< FLevelInfo > AdvancedLevelInformation;
	UPROPERTY( BlueprintReadOnly, EditAnywhere, Category = "Game Data" ) TArray< FUpgradeInfo > UpgradeInformation;
};
