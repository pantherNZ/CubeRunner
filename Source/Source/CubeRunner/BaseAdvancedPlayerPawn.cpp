// Fill out your copyright notice in the Description page of Project Settings.

#include "BaseAdvancedPlayerPawn.h"
#include "CubeRunner.h"
#include "CubeSingletonDataLibrary.h"
#include "Kismet/KismetMathLibrary.h"

ABaseAdvancedPlayerPawn::ABaseAdvancedPlayerPawn( const class FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, JumpVelocity( 1000.0f )
	, JumpCooldownMs( 4000.0f )
	, AddedForwardVelocity( 0.0f )
	, AddedForwardFriction( 0.98f )
	, AddedForwardAcceleration( 3000.0f )
	, AddedForwardMaxSpeed( 500.0f )
{

}

void ABaseAdvancedPlayerPawn::SetupPlayerInputComponent( class UInputComponent* InputComp )
{
	Super::SetupPlayerInputComponent( InputComp );

	InputComp->BindAxis( "MoveForwards", this, &ABaseAdvancedPlayerPawn::MoveForwards );
	InputComp->BindAction( "Jump", EInputEvent::IE_Pressed, this, &ABaseAdvancedPlayerPawn::JumpInput );
	//InputComp->BindVectorAxis( "Gravity", this, &ABaseAdvancedPlayerPawn::Gravity );
}

void ABaseAdvancedPlayerPawn::MoveForwards( float AxisValue )
{
	if( AxisValue != 0.0f && !DisableMovement )
	{
		AxisValue = FMath::Clamp( AxisValue, -1.0f, 1.0f );

		if( CurrentUpgrade == EUpgradeType::EUT_INVERT_CONTROLS )
			AxisValue *= -1.0f;

		if( IsAlive )
		{
			// With the advanced forward / backward movement, either direction causes your strafe max speed to decrease (not increase if going faster)
			const float DeltaTime = GetWorld()->DeltaTimeSeconds;
			auto Increase = AddedForwardAcceleration * AxisValue * ( IsValid( CurrentTurnFloorPiece ) ? 0.8f : 1.0f );
			AddedForwardVelocity = FMath::Clamp( AddedForwardVelocity + Increase * DeltaTime, -AddedForwardMaxSpeed * 0.8f, AddedForwardMaxSpeed );
			UpdateStrafeMaxSpeed();
		}
	}
}

void ABaseAdvancedPlayerPawn::JumpInput()
{
	if( InputMode != EInputMode::EIM_SCREEN_BUTTONS )
		Jump();
}

void ABaseAdvancedPlayerPawn::Jump()
{
	if( !JumpTimerHandle.IsValid() && StartTimer == 0.0f && IsAlive && !DisableMovement )
	{
		// Can only jump when you are closish to the ground
		if( FloorToPawnDistance > 0 && FloorToPawnDistance < HoverHeight + 25.0f )
		{
			HoverAcceleration = 0.0f;
			HoverVelocity += JumpVelocity;
			GetWorld()->GetTimerManager().SetTimer( JumpTimerHandle, this, &ABaseAdvancedPlayerPawn::JumpReset, JumpCooldownMs / 1000.0f );
		}
	}
}

void ABaseAdvancedPlayerPawn::JumpReset()
{
	JumpTimerHandle.Invalidate();
}

void ABaseAdvancedPlayerPawn::ProcessMovement( float DeltaTime )
{
	ABasePlayerPawn::ProcessMovement( DeltaTime );

	if( !IsValid( CurrentTurnFloorPiece ) )
	{
		const auto Forward = GetActorForwardVector() * AddedForwardVelocity;
		TotalDistanceTravelled += AddedForwardVelocity * DeltaTime;
		AddActorWorldOffset( Forward * DeltaTime );
	}
}

void ABaseAdvancedPlayerPawn::Gravity( FVector Gravity )
{
	ABasePlayerPawn::Gravity( Gravity );

	if( InputMode == EInputMode::EIM_GYROSCOPIC && !DisableMovement )
	{
		if( Gravity.Size() != 0.0f )
		{
			const auto NormalisedAngle = FMath::Min( 1.0f, FMath::Max( -1.0f, ( Gravity.Z + 4.5f ) / 4.0f ) );
			MoveForwards( FMath::Min( 1.0f, FMath::Max( -1.0f, NormalisedAngle * RotationRateSensitivity ) ) );
		}
	}
}

void ABaseAdvancedPlayerPawn::ApplyFriction()
{
	ABasePlayerPawn::ApplyFriction();

	if( !IsValid( CurrentTurnFloorPiece ) )
	{
		AddedForwardVelocity *= AddedForwardFriction;

		if( FMath::Abs( AddedForwardVelocity ) <= 0.1f )
			AddedForwardVelocity = 0.0f;
	}
}

void ABaseAdvancedPlayerPawn::UpdateStrafeMaxSpeed()
{
	const float Percent = ( ForwardSpeed - FMath::Abs( AddedForwardVelocity * 1.5f ) ) / ForwardBaseSpeed;
	StrafeMaxSpeed = StrafeBaseMaxSpeed * Percent;
	StrafeAcceleration = StrafeBaseAcceleration * Percent;
}

void ABaseAdvancedPlayerPawn::BeginTurn( ABaseTurnFloorPiece* TurnPiece )
{
	TurnPiece->BeginTurn( GetActorLocation(), ForwardSpeed + AddedForwardVelocity );
	CurrentTurnFloorPiece = TurnPiece;
}
