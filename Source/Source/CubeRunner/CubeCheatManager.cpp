// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeCheatManager.h"
#include "CubeRunner.h"
#include "BasePlayerPawn.h"
#include "CubeGameInstance.h"
#include "CubeSaveGame.h"
#include "CubeRunnerGameMode.h"
#include "CubeSingletonDataLibrary.h"

void UCubeCheatManager::SetSpeed( int32 Speed )
{
	if( auto* PlayerPawn = Cast< ABasePlayerPawn >( UGameplayStatics::GetPlayerController( GetWorld(), 0 )->GetPawn() ) )
		PlayerPawn->SetSpeed( Speed );
}

void UCubeCheatManager::AddSpeed( int32 Speed )
{
	if( auto* PlayerPawn = Cast< ABasePlayerPawn >( UGameplayStatics::GetPlayerController( GetWorld(), 0 )->GetPawn() ) )
		PlayerPawn->SetSpeed( PlayerPawn->ForwardSpeed + Speed );
}

void UCubeCheatManager::CompleteCurrentLevel()
{
	// Complete level
	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
	GameInstance->CompleteLevel( GameInstance->ClassicPlayerMode, GameInstance->LevelIndex );
	Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) )->GameEnd( EGameEndState::EGES_SUCCESS );
}

void UCubeCheatManager::CompleteLevel( int32 Level )
{
	// Complete level
	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
	GameInstance->CompleteLevel( GameInstance->ClassicPlayerMode, Level );
	Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) )->ForceLevelUIReload();
}

void UCubeCheatManager::CompleteLevels()
{
	// Complete levels
	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
	auto CompletedLevels = GameInstance->LoadCompletedLevels( GameInstance->ClassicPlayerMode );
	const auto TotalLevels = UCubeSingletonDataLibrary::GetTotalLevels( GameInstance->ClassicPlayerMode );

	for( int32 i = 0; i < TotalLevels; ++i )
		CompletedLevels.AddUnique( i );

	GameInstance->SaveCompletedLevels( GameInstance->ClassicPlayerMode, CompletedLevels );

	Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) )->ForceLevelUIReload();
}

void UCubeCheatManager::UncompleteLevels()
{
	// Un-complete levels
	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
	GameInstance->SaveCompletedLevels( GameInstance->ClassicPlayerMode, TArray< int32 >() );

	Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) )->ForceLevelUIReload();
}
