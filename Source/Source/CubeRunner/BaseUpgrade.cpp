// Fill out your copyright notice in the Description page of Project Settings.
#include "BaseUpgrade.h"
#include "CubeRunner.h"
#include "CubeSingletonDataLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "CubeDataSingleton.h"

// Sets default values
ABaseUpgrade::ABaseUpgrade( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, UpgradeType( EUpgradeType::EUT_UPGRADE_MAX_TOTAL )
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABaseUpgrade::BeginPlay()
{
	TPair< int32, int32 > range( 0, ( int32 )EUpgradeType::EUT_UPGRADE_MAX_TOTAL );
	const auto* Data = UCubeSingletonDataLibrary::GetGameData();

	if ( Data->UpgradeIsNegativeEffectChancePercent > FMath::RandRange( 0, 99 ) )
		range.Get<0>() = ( int32 )EUpgradeType::EUT_SPEED_FAST;
	else
		range.Get<1>() = ( int32 )EUpgradeType::EUT_SPEED_FAST;

	const auto Roll = FMath::FRand() * 100.0f;
	auto Total = 0.0f;

	for ( int32 i = range.Get<0>(); i < range.Get<1>(); ++i )
	{
		if( ( int32 )UpgradeType >= Data->UpgradeInformation.Num() )
		{
			UpgradeType = static_cast< EUpgradeType >( i );
			UCubeSingletonDataLibrary::CustomLog( "Upgrade index " + FString::FromInt( i ) + " doesn't have valid UpgradeInformation" );
			return;
		}

		Total += Data->UpgradeInformation[i].SpawnChance;

		if ( Roll <= Total )
		{
			UpgradeType = static_cast< EUpgradeType >( i );
			break;
		}
	}

	UCubeSingletonDataLibrary::CustomLog( "Upgrade spawned: " + FString::FromInt( static_cast< int32 >( UpgradeType ) ) );

	Super::BeginPlay();
}

FString ABaseUpgrade::GetUpgradeDisplayName()
{
	const auto* Data = UCubeSingletonDataLibrary::GetGameData();

	if( ( int32 )UpgradeType >= Data->UpgradeInformation.Num() || ( int32 )UpgradeType < 0U )
	{
		UCubeSingletonDataLibrary::CustomLog( "GetUpgradeDisplayName: Failed to get UpgradeInformation for enum index: " + FString::FromInt( ( int32 )UpgradeType ), LogDisplayType::Error );
		return "Unspecified Upgrade";
	}
	
	return Data->UpgradeInformation[( int32 )UpgradeType].Name;
}

