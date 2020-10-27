// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameDataAssets.h"
#include "CubeSingletonDataLibrary.generated.h"

UENUM( BlueprintType )
enum class LogDisplayType : uint8
{
	Gameplay,
	Warn,
	Error,
};

UCLASS()
class CUBERUNNER_API UCubeSingletonDataLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UCubeSingletonDataLibrary( const FObjectInitializer& ObjectInitializer );

	UFUNCTION( BlueprintPure, Category = "Utility" )
	static UGameDataAssets* GetGameData();

	UFUNCTION( BlueprintPure, Category = "Utility" )
	static UCubeDataSingleton* GetSingletonGameData();

	UFUNCTION( BlueprintPure, Category = "Utility" )
	static int32 GetDifficultyLevels( const bool ClassicMode );

	UFUNCTION( BlueprintPure, Category = "Utility" )
	static int32 GetTotalLevels( const bool ClassicMode );

	UFUNCTION( BlueprintPure, Category = "Utility" )
	static bool IsWithEditor();

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	static void CustomLog( FString CustomOutput, LogDisplayType CustomLogType = LogDisplayType::Gameplay );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	static FString GetOnlineAccountID( APlayerController* PlayerController );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	static FVector GetRainbowColour( int32 value, int32 max );
};
