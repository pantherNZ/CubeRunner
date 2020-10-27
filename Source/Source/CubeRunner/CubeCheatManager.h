// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CheatManager.h"
#include "CubeCheatManager.generated.h"

UCLASS()
class CUBERUNNER_API UCubeCheatManager : public UCheatManager
{
	GENERATED_BODY()
	
public:
	UFUNCTION( exec )
	void SetSpeed( int32 Speed );
	
	UFUNCTION( exec )
	void AddSpeed( int32 Increase );

	UFUNCTION( exec )
	void CompleteCurrentLevel();

	UFUNCTION( exec )
	void CompleteLevel( int32 Level );

	UFUNCTION( exec )
	void CompleteLevels();

	UFUNCTION( exec )
	void UncompleteLevels();
};
