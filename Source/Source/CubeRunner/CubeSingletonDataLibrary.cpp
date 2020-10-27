// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeSingletonDataLibrary.h"
#include "CubeRunner.h"
#include "CubeDataSingleton.h"
#include "GameFramework/PlayerState.h"

UCubeSingletonDataLibrary::UCubeSingletonDataLibrary( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{

}

UCubeDataSingleton* UCubeSingletonDataLibrary::GetSingletonGameData()
{
	return Cast<UCubeDataSingleton>( GEngine->GameSingleton );
}

int32 UCubeSingletonDataLibrary::GetDifficultyLevels( const bool ClassicMode )
{
	return ClassicMode ? GetGameData()->ClassicDifficultyLevels : GetGameData()->AdvancedDifficultyLevels;
}

int32 UCubeSingletonDataLibrary::GetTotalLevels( const bool ClassicMode )
{
	return GetDifficultyLevels( ClassicMode ) * GetGameData()->LevelsPerDifficulty;
}

UGameDataAssets* UCubeSingletonDataLibrary::GetGameData()
{
	return GetSingletonGameData()->GameData;
}

bool UCubeSingletonDataLibrary::IsWithEditor()
{
	return GIsEditor;
}

void UCubeSingletonDataLibrary::CustomLog( FString CustomOutput, LogDisplayType CustomLogType /*= Gameplay*/ )
{
	if( GetSingletonGameData() && GetSingletonGameData()->EnableDebugLogging )
	{
		if( CustomLogType == LogDisplayType::Gameplay )
		{
			GEngine->AddOnScreenDebugMessage( -1, 5.0f, FColor::Green, CustomOutput );
			UE_LOG( LogTemp, Display, TEXT( "%s" ), *CustomOutput );
		}
		else if( CustomLogType == LogDisplayType::Warn )
		{
			GEngine->AddOnScreenDebugMessage( -1, 5.0f, FColor::Yellow, CustomOutput );
			UE_LOG( LogTemp, Warning, TEXT( "%s" ), *CustomOutput );
		}
	}

	if( CustomLogType == LogDisplayType::Error )
	{
		GEngine->AddOnScreenDebugMessage( -1, 5.0f, FColor::Red, CustomOutput );
		UE_LOG( LogTemp, Error, TEXT( "%s" ), *CustomOutput );
	}
}

FString UCubeSingletonDataLibrary::GetOnlineAccountID( APlayerController* PlayerController )
{
	if( PlayerController && PlayerController->PlayerState && PlayerController->PlayerState->UniqueId.IsValid() )
	{
		return PlayerController->PlayerState->UniqueId->GetHexEncodedString();
	}
	return FString();
}

FVector UCubeSingletonDataLibrary::GetRainbowColour( int32 value, int32 max )
{
	float inc = 6.0 / max;
	float x = value * inc;
	float r = 0.0f;
	float g = 0.0f;
	float b = 0.0f;

	if ( ( 0 <= x && x <= 1 ) || ( 5 <= x && x <= 6 ) ) r = 1.0f;
	else if ( 4 <= x && x <= 5 ) r = x - 4;
	else if ( 1 <= x && x <= 2 ) r = 1.0f - ( x - 1 );
	if ( 1 <= x && x <= 3 ) g = 1.0f;
	else if ( 0 <= x && x <= 1 ) g = x - 0;
	else if ( 3 <= x && x <= 4 ) g = 1.0f - ( x - 3 );
	if ( 3 <= x && x <= 5 ) b = 1.0f;
	else if ( 2 <= x && x <= 3 ) b = x - 2;
	else if ( 5 <= x && x <= 6 ) b = 1.0f - ( x - 5 );

	return FVector( r, g, b );
}
