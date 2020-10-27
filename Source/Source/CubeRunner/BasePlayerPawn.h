// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "BaseTurnFloorPiece.h"
#include "BaseWallPiece.h"
#include "BaseUpgrade.h"
#include "BasePlayerPawn.generated.h"

UENUM( BlueprintType )
enum class EInputMode : uint8
{
	EIM_KEYBOARD UMETA( DisplayName = "Keyboard" ),
	EIM_SCREEN_BUTTONS UMETA( DisplayName = "Screen Buttons" ),
	EIM_GYROSCOPIC UMETA( DisplayName = "Mobile Gyroscopic" ),
};

UCLASS()
class CUBERUNNER_API ABasePlayerPawn : public APawn
{
	GENERATED_BODY()

	// Functions
public:
	ABasePlayerPawn( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent( class UInputComponent* InputComponent ) override;

	UFUNCTION( BlueprintImplementableEvent, Category = "Events" )
	void StartTimerComplete();

	void SetInputMode( EInputMode NewInputMode );

	UFUNCTION( BlueprintCallable, Category = "Input" )
	void MoveSidewaysInputButtons( float AxisValue );

	UFUNCTION()
	void MoveSidewaysInput( float AxisValue );

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void Explode( bool force = false );

	UFUNCTION( BlueprintCallable, Category = "Events" )
	void AddUpgrade( ABaseUpgrade* Upgrade );

	UFUNCTION()
	void OnMeshOverlapBegin( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult );

	UFUNCTION()
	void OnMeshOverlapEnd( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex );

	// Utility
	void IncreaseSpeed( const int32 Speed, const float DeltaTime = 1.0f );
	void SetSpeed( const int32 Speed );
	virtual void UpdateStrafeMaxSpeed();

protected:
	virtual void ProcessMovement( float DeltaTime );
	virtual void ApplyFriction();
	virtual void BeginTurn( ABaseTurnFloorPiece* TurnPiece );

	// Input
	virtual void Gravity( FVector Gravity );

	void MoveSideways( float AxisValue );
	void ProcessUpgradeTimer( float DeltaTime );
	void ProcessStrafeRoll( float DeltaTime );
	void ProcessHeightTracing( float DeltaTime );
	void RemoveUpgrade( EUpgradeType Type );

	// Members
public:
	// Components
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class USceneComponent* Root;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class USpringArmComponent* CameraBoom;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UCameraComponent* Camera;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = Members, meta = ( AllowPrivateAccess = "true" ) ) class UStaticMeshComponent* Mesh;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float ExplosionSize;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float RollSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float RollMax;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) float CurrentRoll;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float HoverHeight;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float ThrustStrength;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float Damping;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float FloorRotateInterpSpeed;

	// If this is set it will disable movement & also be rendered on the HUD
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) float StartTimer;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) bool IsAlive;
	// Used so that the pawn can keep moving once you complete the level, then stops after 5 seconds
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) bool DisableMovement;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) ABaseTurnFloorPiece* CurrentTurnFloorPiece;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) ABaseWallPiece* CurrentWallPiece;

	float StrafeBaseMaxSpeed;
	float ForwardBaseSpeed;
	float StrafeBaseAcceleration;
	bool TouchingWallPiece;

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float StrafeMaxSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float StrafeVelocity;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float StrafeFriction;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float StrafeAcceleration;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float ForwardSpeed;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float ForwardSpeedIncrease;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) EUpgradeType CurrentUpgrade;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float TotalDistanceTravelled;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float HoverFriction;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float HoverVelocity;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Stats" ) float HoverAcceleration;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) float RotationRateSensitivity;
	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Stats" ) EInputMode InputMode;

	float UpgradeTimer;
	float PreviousForwardSpeed;
	bool HasFirstCollision;
	float FloorToPawnDistance;
};