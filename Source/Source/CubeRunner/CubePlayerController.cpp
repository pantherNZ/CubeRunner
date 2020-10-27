// Fill out your copyright notice in the Description page of Project Settings.

#include "CubePlayerController.h"
#include "CubeRunner.h"
#include "CubeCheatManager.h"

ACubePlayerController::ACubePlayerController( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{
	CheatClass = UCubeCheatManager::StaticClass();
}
