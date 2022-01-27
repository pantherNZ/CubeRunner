// Fill out your copyright notice in the Description page of Project Settings.

#include "GameDataAssets.h"

UGameDataAssets::UGameDataAssets( const FObjectInitializer& ObjectInitializer ) 
	: Super( ObjectInitializer )
	, RandomisedFloorPieceBPClass( nullptr )
	, DefaultUpgradeBPClass( nullptr )
	, ClassicCubeObstacleBPClass( nullptr )
	, AdvancedCubeObstacleBPClass( nullptr )
	, UpgradeParticle( nullptr )
	, ExplosionParticle( nullptr )
	, LevelsPerDifficulty( 4 )
	, ClassicDifficultyLevels( 4 )
	, AdvancedDifficultyLevels( 4 )
	, UpgradeSpawnChancePerPiecePercent( 10 )
	, UpgradeIsNegativeEffectChancePercent( 20 )
{

}