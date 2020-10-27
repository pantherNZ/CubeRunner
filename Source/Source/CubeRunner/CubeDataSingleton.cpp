// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeDataSingleton.h"
#include "CubeRunner.h"
#include "BaseUpgrade.h"
#include "CubeSingletonDataLibrary.h"

UCubeDataSingleton::UCubeDataSingleton( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, EnableDebugLogging( true )
{

}

void UCubeDataSingleton::CorrectUpgradeSpawnOdds()
{
	float Total = 0.0f;

	for ( int32 i = 0; i < GameData->UpgradeInformation.Num(); ++i )
		Total += GameData->UpgradeInformation[i].SpawnChance;

	for ( int32 i = 0; i < GameData->UpgradeInformation.Num(); ++i )
		GameData->UpgradeInformation[i].SpawnChance *= 100.0f / Total;
}

void UCubeDataSingleton::PreloadGameObjects()
{
	UCubeSingletonDataLibrary::CustomLog( "Requesting preloading for " + FString::FromInt( GameData->AssetsToLoad.Num() ) + " assets", LogDisplayType::Gameplay );

	for ( auto& asset : GameData->AssetsToLoad )
	{
		auto* object = AssetLoader.LoadSynchronous( asset, true );
		LoadedObjectHandles.Add( object );
	}

	if( !ObjectLibrary )
	{
		ObjectLibrary = UObjectLibrary::CreateLibrary( UObject::StaticClass(), true, GIsEditor );
		ObjectLibrary->AddToRoot();
	}
	
	//const auto Count = ObjectLibrary->LoadAssetsFromPaths( FoldersToLoad );
	//UCubeSingletonDataLibrary::CustomLog( "Preloaded " + FString::FromInt( Count ) + " assets", LogDisplayType::Gameplay );

	for( auto Folder : GameData->FoldersToLoad )
	{
		const auto Count = ObjectLibrary->LoadAssetDataFromPath( Folder );
		UCubeSingletonDataLibrary::CustomLog( "Requesting preloading asset data from path \"" + Folder + "\": " + FString::FromInt( Count ) + " assets", LogDisplayType::Gameplay );

		TArray<FAssetData> AssetDatas;
		ObjectLibrary->GetAssetDataList( AssetDatas );

		UObject* Asset;

		for( int32 i = 0; i < AssetDatas.Num(); ++i )
		{
			FAssetData& AssetData = AssetDatas[i];
			Asset = AssetData.GetAsset();
			if( Asset )
			{
				LoadedObjectHandles.Add( Asset ); 
			}
		}
	}

	const auto Total = ObjectLibrary->LoadAssetsFromAssetData();
	UCubeSingletonDataLibrary::CustomLog( "Preloaded " + FString::FromInt( LoadedObjectHandles.Num() ) + " assets", LogDisplayType::Gameplay );
}

bool UCubeDataSingleton::IsPreloadingFinished()
{
	return HaveAsyncObjectsFinishedLoading && ObjectLibrary->IsLibraryFullyLoaded();
}