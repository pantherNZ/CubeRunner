// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "BasePlayerPawn.h"
#include "BaseAdvancedPlayerPawn.generated.h"

UCLASS()
class CUBERUNNER_API ABaseAdvancedPlayerPawn : public ABasePlayerPawn
{
	GENERATED_BODY()

	// Functions
public:
	ABaseAdvancedPlayerPawn( const FObjectInitializer& ObjectInitializer );

	void SetupPlayerInputComponent( class UInputComponent* InputComp ) override;

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void MoveForwards( float AxisValue );

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void Jump();

	UFUNCTION()
	void JumpInput();

	UFUNCTION()
	void JumpReset();

protected:
	void Gravity( FVector Gravity ) override;
	void ProcessMovement( float DeltaTime ) override;
	void ApplyFriction() override;
	void UpdateStrafeMaxSpeed() override;
	void BeginTurn( ABaseTurnFloorPiece* TurnPiece ) override;

	// Members
public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float AddedForwardMaxSpeed;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float AddedForwardVelocity;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float AddedForwardFriction;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float AddedForwardAcceleration;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float JumpVelocity;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) float JumpCooldownMs;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) FTimerHandle JumpTimerHandle;
};