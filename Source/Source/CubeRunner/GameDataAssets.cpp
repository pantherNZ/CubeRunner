// Fill out your copyright notice in the Description page of Project Settings.

#include "GameDataAssets.h"

UGameDataAssets::UGameDataAssets( const FObjectInitializer& ObjectInitializer ) 
	: Super( ObjectInitializer )
	, LevelsPerDifficulty( 4 )
	, ClassicDifficultyLevels( 4 )
	, AdvancedDifficultyLevels( 4 )
	, RandomisedFloorPieceBPClass( nullptr )
	, UpgradeParticle( nullptr )
	, ExplosionParticle( nullptr )
	, DefaultUpgradeBPClass( nullptr )
	, ClassicCubeObstacleBPClass( nullptr )
	, AdvancedCubeObstacleBPClass( nullptr )
	, UpgradeSpawnChancePerPiecePercent( 10 )
	, UpgradeIsNegativeEffectChancePercent( 20 )
{

}