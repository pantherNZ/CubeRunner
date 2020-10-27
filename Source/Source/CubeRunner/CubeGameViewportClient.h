// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "CubeGameViewportClient.generated.h"

UCLASS( )
class CUBERUNNER_API UCubeGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY( )

public:
	virtual void LostFocus( FViewport* Viewport ) override;
	virtual void ReceivedFocus( FViewport* Viewport ) override;
};