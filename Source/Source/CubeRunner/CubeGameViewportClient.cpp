// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeGameViewportClient.h"
#include "CubeRunner.h"
#include "BasePlayerPawn.h"
#include "CubeGameInstance.h"

void UCubeGameViewportClient::LostFocus( FViewport* _Viewport )
{
	UGameViewportClient::LostFocus( _Viewport );
	static_cast<UCubeGameInstance*>( GameInstance )->OnLostFocus.Broadcast();
}

void UCubeGameViewportClient::ReceivedFocus( FViewport* _Viewport )
{
	UGameViewportClient::ReceivedFocus( _Viewport );
}

