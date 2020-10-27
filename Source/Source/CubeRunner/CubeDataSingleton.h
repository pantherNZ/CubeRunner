// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "Engine/StreamableManager.h"
#include "Engine/ObjectLibrary.h"
#include "GameDataAssets.h"
#include "CubeDataSingleton.generated.h"

UCLASS( Blueprintable )
class CUBERUNNER_API UCubeDataSingleton : public UObject
{
	GENERATED_BODY()
	
public:
	// Methods
	UCubeDataSingleton( const FObjectInitializer& ObjectInitializer );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void CorrectUpgradeSpawnOdds();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	void PreloadGameObjects();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	bool IsPreloadingFinished();

	// Members
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Game Data" ) UGameDataAssets* GameData;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Game Data" ) bool EnableDebugLogging;

private:

	bool HaveAsyncObjectsFinishedLoading = false;
	FStreamableManager AssetLoader;
	UObjectLibrary* ObjectLibrary = nullptr;
	TArray< UObject* > LoadedObjectHandles;
};
