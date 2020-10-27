// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeGameInstance.h"
#include "CubeRunner.h"
#include "CubeSaveGame.h"
#include "CubeDataSingleton.h"
#include "CubeSingletonDataLibrary.h"

UCubeGameInstance::UCubeGameInstance( const FObjectInitializer& ObjectInitializer )
{

}

void UCubeGameInstance::Init()
{
	LoginChangedHandle = FCoreDelegates::OnUserLoginChangedEvent.AddUObject( this, &UCubeGameInstance::OnLoginChanged );
	EnteringForegroundHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject( this, &UCubeGameInstance::OnEnteringForeground );
	EnteringBackgroundHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject( this, &UCubeGameInstance::OnEnteringBackground );

	Super::Init();
}

void UCubeGameInstance::Shutdown()
{
	FCoreDelegates::OnUserLoginChangedEvent.Remove( LoginChangedHandle );
	FCoreDelegates::OnUserLoginChangedEvent.Remove( EnteringForegroundHandle );
	FCoreDelegates::OnUserLoginChangedEvent.Remove( EnteringBackgroundHandle );

	Super::Shutdown();
}

float UCubeGameInstance::GetOptionValueScaled( float Value, float Min, float Max )
{
	return Min + ( Max - Min ) * Value;
}

float UCubeGameInstance::GetOptionValueRaw( float Value, float Min, float Max )
{
	return ( Value - Min ) / ( Max - Min );
}

void UCubeGameInstance::CompleteLevel( const bool ClassicMode, const int32 Level )
{
	if( !CheckSaveGame() )
		return;

	auto& LevelsArray = ClassicMode ? InstanceSaveGameData->CompletedLevelsClassic : InstanceSaveGameData->CompletedLevelsAdvanced;

	if( !LevelsArray.Contains( Level ) )
	{
		LevelsComplete++;
		LevelsArray.Add( Level );
		SaveGame();
	}
}

TArray< int32 > UCubeGameInstance::LoadCompletedLevels( const bool ClassicMode )
{
	if( !CheckSaveGame() )
		return TArray< int32 >();

	return ClassicMode ? InstanceSaveGameData->CompletedLevelsClassic : InstanceSaveGameData->CompletedLevelsAdvanced;
}

TArray< int32 > UCubeGameInstance::LoadUncompletedLevels( const bool ClassicMode )
{
	if( !CheckSaveGame() )
		return TArray< int32 >();

	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );
	const auto TotalLevels = UCubeSingletonDataLibrary::GetTotalLevels( ClassicMode );

	TArray< int32 > UncompletedLevels;
	for( int32 i = 0; i < TotalLevels; ++i )
		UncompletedLevels.Add( i );

	TArray< int32 > CompletedLevels = LoadCompletedLevels( ClassicMode );
	for( auto Level : CompletedLevels )
		UncompletedLevels.Remove( Level );

	return UncompletedLevels;
}

TArray< int32 > UCubeGameInstance::LoadCompletedLevelsOfDifficulty( const bool ClassicMode, const int32 Difficulty )
{
	if( !CheckSaveGame() )
		return TArray< int32 >();

	auto Levels = LoadCompletedLevels( ClassicMode );
	TArray< int32 > LevelsOfDifficulty;

	for( auto& Level : Levels )
		if( int32( Level / UCubeSingletonDataLibrary::GetDifficultyLevels( ClassicMode ) ) == Difficulty )
			LevelsOfDifficulty.Add( Level );

	return LevelsOfDifficulty;
}

TArray< int32 > UCubeGameInstance::LoadUncompletedLevelsOfDifficulty( const bool ClassicMode, const int32 Difficulty )
{
	if( !CheckSaveGame() )
		return TArray< int32 >();

	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );
	const auto TotalLevels = UCubeSingletonDataLibrary::GetTotalLevels( ClassicMode );

	TArray< int32 > UncompletedLevels;
	for( int32 i = 0; i < TotalLevels; ++i )
		UncompletedLevels.Add( i );

	TArray< int32 > CompletedLevels = LoadCompletedLevelsOfDifficulty( ClassicMode, Difficulty );
	for( auto Level : CompletedLevels )
		UncompletedLevels.Remove( Level );

	return UncompletedLevels;
}

void UCubeGameInstance::SaveCompletedLevels( const bool ClassicMode, const TArray< int32 >& Levels )
{
	if( !CheckSaveGame() )
		return;

	if( ClassicMode )
		InstanceSaveGameData->CompletedLevelsClassic = Levels;
	else
		InstanceSaveGameData->CompletedLevelsAdvanced = Levels;
	SaveGame();
}

bool UCubeGameInstance::HasUnlockedDifficulty( const TArray< int32 >& Levels, const int32 Difficulty )
{
	if( Difficulty == 0 )
		return true;

	if( Levels.Num() == 0 )
		return false;

	const auto* GameData = UCubeSingletonDataLibrary::GetGameData();

	int32 Counter = 0;
	int32 Counter2 = 0;

	for( auto Level : Levels )
	{
		if( Level / GameData->LevelsPerDifficulty == Difficulty - 1 )
			Counter++;

		if( Difficulty > 1 && Level / GameData->LevelsPerDifficulty == Difficulty - 2 )
			Counter2++;
	}

	// Must have 2 of current difficulty level & 4 or more of the previous one
	return Counter >= GameData->LevelsPerDifficulty / 2 && ( Difficulty == 1 || Counter2 >= GameData->LevelsPerDifficulty );
}

void UCubeGameInstance::SaveGame()
{
	if( !CheckSaveGame() )
		return;

	UGameplayStatics::SaveGameToSlot( InstanceSaveGameData, GetSaveSlotName(), 0 );
}

bool UCubeGameInstance::LoadCustomValue( FString Key, float& LoadedValue )
{
	if( !CheckSaveGame() )
		return false;

	if( const auto* ValuePointer = InstanceSaveGameData->CustomData.Find( Key ) )
	{
		LoadedValue = *ValuePointer;
		return true;
	}
	return false;
}

bool UCubeGameInstance::LoadCustomBool( FString Key, bool& LoadedValue )
{
	if( !CheckSaveGame() )
		return false;

	if( const auto* ValuePointer = InstanceSaveGameData->CustomData.Find( Key ) )
	{
		LoadedValue = ( *ValuePointer ) != 0.0f;
		return true;
	}
	return false;
}

void UCubeGameInstance::SaveCustomValue( FString Key, float Value )
{
	if( !CheckSaveGame() )
		return;

	InstanceSaveGameData->CustomData.FindOrAdd( Key ) = Value;
}

void UCubeGameInstance::SaveCustomBool( FString Key, bool Value )
{
	if( !CheckSaveGame() )
		return;

	InstanceSaveGameData->CustomData.FindOrAdd( Key ) = ( float )Value;
}

void UCubeGameInstance::ClearCustomValue( FString Key )
{
	if( !CheckSaveGame() )
		return;

	InstanceSaveGameData->CustomData.Remove( Key );
}

void UCubeGameInstance::RegisterOnlineID( FString NewOnlineID )
{
	SaveGamePrefix = NewOnlineID;
	InitSaveGameSlot();
}

void UCubeGameInstance::InitSaveGameSlot()
{
	const FString SaveSlotName = GetSaveSlotName();
	if( !UGameplayStatics::DoesSaveGameExist( SaveSlotName, 0 ) )
	{
		// Clear default save file, if it exists.
		if( UGameplayStatics::DoesSaveGameExist( DefaultSaveGameSlot, 0 ) )
		{
			UGameplayStatics::DeleteGameInSlot( DefaultSaveGameSlot, 0 );
		}
		// If we have no save object, create one.
		if( InstanceSaveGameData == nullptr )
		{
			// We're either not logged in with an Online ID, or we have no save data to transfer over (usually, this indicates program startup).
			InstanceSaveGameData = Cast<UCubeSaveGame>( UGameplayStatics::CreateSaveGameObject( UCubeSaveGame::StaticClass() ) );
		}
		UGameplayStatics::SaveGameToSlot( InstanceSaveGameData, SaveSlotName, 0 );
	}
	else
	{
		InstanceSaveGameData = Cast<UCubeSaveGame>( UGameplayStatics::LoadGameFromSlot( SaveSlotName, 0 ) );
	}
	
	CheckSaveGame();
}

FString UCubeGameInstance::GetSaveSlotName() const
{
	return SaveGamePrefix + DefaultSaveGameSlot;
}

bool UCubeGameInstance::CheckSaveGame() const
{
	if( !InstanceSaveGameData )
	{
		UCubeSingletonDataLibrary::CustomLog( "InstanceSaveGameData has not been loaded yet!", LogDisplayType::Error );
		return false;
	}
	return true;
}
