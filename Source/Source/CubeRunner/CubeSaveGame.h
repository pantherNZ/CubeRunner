// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/SaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "CubeGameInstance.h"
#include "CubeSaveGame.generated.h"

UCLASS()
class CUBERUNNER_API UCubeSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UCubeSaveGame( const FObjectInitializer& ObjectInitializer );

	UPROPERTY( VisibleAnywhere, Category = Basic ) TArray< int32 > CompletedLevelsClassic;
	UPROPERTY( VisibleAnywhere, Category = Basic ) TArray< int32 > CompletedLevelsAdvanced;
	UPROPERTY( VisibleAnywhere, Category = Basic ) TArray< float > AchievementProgress;
	UPROPERTY( VisibleAnywhere, Category = Basic ) TMap<FString, float> CustomData;
	UPROPERTY( VisibleAnywhere, Category = Basic ) TMap<FString, float> CustomDataGlobal;
};
