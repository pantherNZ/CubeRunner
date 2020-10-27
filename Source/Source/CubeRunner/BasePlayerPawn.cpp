// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlayerPawn.h"
#include "CubeRunner.h"
#include "BaseObstacle.h"
#include "BaseObstacleComponent.h"
#include "CubeRunnerGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "CubeDataSingleton.h"
#include "BaseTransitionFloorPiece.h"
#include "CubeSingletonDataLibrary.h"
#include "GameFramework/SpringArmComponent.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "BaseTurnFloorPiece.h"

// Sets default values
ABasePlayerPawn::ABasePlayerPawn( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, IsAlive( true )
	, DisableMovement( true )
	, RollSpeed( 70.0f )
	, RollMax( 10.0f )
	, CurrentRoll( 0.0f )
	, CurrentTurnFloorPiece( nullptr )
	, CurrentWallPiece( nullptr )
	, HoverHeight( 75.0f )
	, FloorRotateInterpSpeed( 4.0f )
	, ForwardSpeed( 800.0f )
	, StrafeMaxSpeed( 550.0f )
	, StrafeVelocity( 0.0f )
	, StrafeFriction( 0.98f )
	, HoverFriction( 0.98f )
	, StrafeAcceleration( 2500.0f )
	, ForwardSpeedIncrease( 20.0f )
	, ThrustStrength( 30.0f )
	, TouchingWallPiece( false )
	, CurrentUpgrade( EUpgradeType::EUT_UPGRADE_NONE )
	, UpgradeTimer( 0.0f )
	, PreviousForwardSpeed( 0.0f )
	, Damping( 0.6f )
	, StartTimer( -1.0f )
	, TotalDistanceTravelled( 0.0f )
	, RotationRateSensitivity( 0.005f )
	, InputMode( EInputMode::EIM_KEYBOARD )
	, HasFirstCollision( false )
	, FloorToPawnDistance( 0.0f )
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Setup components
	Root = CreateDefaultSubobject<USceneComponent>( TEXT( "Root" ) );
	RootComponent = Root;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>( TEXT( "Mesh" ) );
	Mesh->SetupAttachment( Root );
	Mesh->bMultiBodyOverlap = true;
	Mesh->bGenerateOverlapEvents = true;
	Mesh->SetCollisionEnabled( ECollisionEnabled::QueryAndPhysics );
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>( TEXT( "CameraBoom" ) );
	CameraBoom->SetupAttachment( Mesh );
	Camera = CreateDefaultSubobject<UCameraComponent>( TEXT( "FollowCamera" ) );
	Camera->SetupAttachment( CameraBoom, CameraBoom->GetAttachSocketName() );
}

// Called when the game starts or when spawned
void ABasePlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	StrafeBaseMaxSpeed = StrafeMaxSpeed;
	ForwardBaseSpeed = ForwardSpeed; 
	StrafeBaseAcceleration = StrafeAcceleration;

	Mesh->OnComponentBeginOverlap.AddDynamic( this, &ABasePlayerPawn::OnMeshOverlapBegin );
	Mesh->OnComponentEndOverlap.AddDynamic( this, &ABasePlayerPawn::OnMeshOverlapEnd );
}

// Called every frame
void ABasePlayerPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
	
	if( StartTimer == 0.0f )
	{
		ProcessHeightTracing( DeltaTime );
	}
	else if( StartTimer > 0.0f )
	{
		StartTimer = FMath::Max( 0.0f, StartTimer - DeltaTime );
		DisableMovement = StartTimer > 0.0f;

		if( StartTimer <= 0.0f )
			StartTimerComplete();
	}

	if( !DisableMovement )
		ProcessMovement( DeltaTime );

	if( IsAlive && !DisableMovement )
	{
		if( InputMode == EInputMode::EIM_KEYBOARD || InputMode == EInputMode::EIM_SCREEN_BUTTONS )
			ProcessStrafeRoll( DeltaTime );

		ProcessUpgradeTimer( DeltaTime );
		ApplyFriction();

		if( GetActorLocation().Z <= -500.0f )
			Explode( true );
	}
}

void ABasePlayerPawn::ProcessUpgradeTimer( float DeltaTime )
{
	if( UpgradeTimer )
	{
		UpgradeTimer -= DeltaTime;

		if( UpgradeTimer <= 0.0f )
		{
			UpgradeTimer = 0.0f;
			RemoveUpgrade( CurrentUpgrade );
			CurrentUpgrade = EUpgradeType::EUT_UPGRADE_NONE;
		}
	}
}

void ABasePlayerPawn::ProcessMovement( float DeltaTime )
{
	if( !IsValid( CurrentTurnFloorPiece ) )
	{
		auto WallCorrection = 0.0f;

		// Wall collision
		if( IsValid( CurrentWallPiece ) )
		{
			StrafeVelocity = ( CurrentWallPiece->LeftWall ? FMath::Max( StrafeVelocity, 0.0f ) : FMath::Min( StrafeVelocity, 0.0f ) );

			//if( TouchingWallPiece )
			//{
			//	const auto WallAngle = CurrentWallPiece->GetActorRotation().Yaw;
			//	// Trig to caculate how much to move the pawn out by the avoid collision
			//	if( ( CurrentWallPiece->LeftWall && WallAngle > 0 ) || ( !CurrentWallPiece->LeftWall && WallAngle < 0 ) )
			//		WallCorrection = FMath::Tan( WallAngle ) * ForwardSpeed;
			//}

			//TouchingWallPiece = true;	
		}
		else
		{
			TouchingWallPiece = false;
		}

		const auto Side = FVector( GetActorRightVector().X, GetActorRightVector().Y, 0.0f ) * ( StrafeVelocity + WallCorrection );
		const auto Forward = GetActorForwardVector() * ForwardSpeed;
		const auto Up = FVector( 0.0f, 0.0f, HoverVelocity );
		TotalDistanceTravelled += ForwardSpeed * DeltaTime;

		AddActorWorldOffset( ( Forward + Side + Up ) * DeltaTime );
		IncreaseSpeed( ForwardSpeedIncrease * ( HoverHeight + 25.0f ? 1.0f : 0.9f ), DeltaTime );
	}
	else
	{
		auto NewTransform = CurrentTurnFloorPiece->GetTurnTargetTransform( StrafeVelocity * DeltaTime );

		auto NewLocation = NewTransform.GetLocation();
		NewLocation.Z = GetActorLocation().Z;
		SetActorLocation( NewLocation );

		AddActorWorldOffset( FVector( 0.0f, 0.0f, HoverVelocity ) * DeltaTime );

		auto NewRotation = GetActorRotation();
		NewRotation.Yaw = NewTransform.GetRotation().Rotator().Yaw;
		SetActorRotation( NewRotation );

		TotalDistanceTravelled += ForwardSpeed * DeltaTime;
	}
}

void ABasePlayerPawn::ProcessStrafeRoll( float DeltaTime )
{
	auto NewRotation = GetActorRotation();
	const auto RollPerTick = RollSpeed * DeltaTime;
	const auto Smoothen = 1.0f;// FMath::Min( 1.0f, FMath::Abs( RollMax - NewRotation.Roll ) / RollMax );

	if( abs( CurrentRoll ) <= 0.1f )
	{
		if( NewRotation.Roll != 0.0f )
		{
			const auto Difference = NewRotation.Roll - RollPerTick * FMath::Sign( NewRotation.Roll ) * FMath::Abs( NewRotation.Roll ) / RollMax / 2.0f;

			if( FMath::Abs( Difference ) <= 0.2f )
				NewRotation.Roll = 0.0f;
			else
				NewRotation.Roll = Difference;
		}
	}
	else if( CurrentRoll < 0.1f )
	{
		if( NewRotation.Roll > -RollMax )
			NewRotation.Roll = FMath::Max( -RollMax, NewRotation.Roll - RollPerTick * Smoothen );
	}
	else
	{
		if( NewRotation.Roll < RollMax )
			NewRotation.Roll = FMath::Min( RollMax, NewRotation.Roll + RollPerTick * Smoothen );
	}

	if( CurrentUpgrade == EUpgradeType::EUT_UPSIDE_DOWN )
		NewRotation.Roll += 180.0f;

	SetActorRotation( NewRotation );
}

void ABasePlayerPawn::ProcessHeightTracing( float DeltaTime )
{
	FHitResult FloorTraceResultFront( ForceInit );
	FHitResult FloorTraceResultBack( ForceInit );

	FCollisionQueryParams FloorTraceParams = FCollisionQueryParams( FName( TEXT( "FloorTrace" ) ), true, this );

	const float Distance = 10000.0f;
	const auto BaseFloorTracePos = Mesh->GetComponentLocation() - FVector( 0.0f, 0.0f, Distance );
	const auto FloorTraceEndPosFront = BaseFloorTracePos + this->GetActorForwardVector() * Distance * 0.1f;
	const auto FloorTraceEndPosBack = BaseFloorTracePos - this->GetActorForwardVector() * Distance * 0.1f;

	GetWorld()->LineTraceSingleByChannel( FloorTraceResultFront, Mesh->GetComponentLocation(), FloorTraceEndPosFront, ECC_WorldStatic, FloorTraceParams );
	//DrawDebugLine( GetWorld(), Mesh->GetComponentLocation() , FloorTraceResultFront.Location, FColor(255, 0, 0), true, -1, 0, 1.0f );

	GetWorld()->LineTraceSingleByChannel( FloorTraceResultBack, Mesh->GetComponentLocation(), FloorTraceEndPosBack, ECC_WorldStatic, FloorTraceParams );
	//DrawDebugLine( GetWorld(), Mesh->GetComponentLocation() , FloorTraceResultBack.Location, FColor(255, 0, 0), true, -1, 0, 1.0f );

	FloorToPawnDistance = FMath::Min( FloorTraceResultFront.Distance, FloorTraceResultBack.Distance );

	FRotator Rotation = GetActorRotation();

	// Hovering code
	if( FloorToPawnDistance > 0 && FloorToPawnDistance < HoverHeight )
	{	
		const auto Thrust = ThrustStrength * ThrustStrength * ( HoverHeight - FloorToPawnDistance );
		const auto Smoothing = HoverVelocity * 2.0f * Damping * ThrustStrength;
		const auto FinalThrust = ( Thrust - Smoothing );// *Mesh->GetMass();// -Gravity;
		HoverVelocity += FinalThrust * FMath::Min( 0.1f, DeltaTime );
		HoverAcceleration = 0.0f;
		const bool UsingFrontTrace = FloorToPawnDistance == FloorTraceResultFront.Distance;
		const auto RotFromZ = UKismetMathLibrary::MakeRotFromZ( UsingFrontTrace ? FloorTraceResultFront.Normal : FloorTraceResultBack.Normal );
		Rotation.Pitch = RotFromZ.Roll * ( UsingFrontTrace ? -1.0f : 1.0f );
	}
	else
	{
		// Fake gravity ;)
		HoverAcceleration += GetWorld()->GetGravityZ() * ( Mesh->IsGravityEnabled() ? 1.0f : 0.0f ) * DeltaTime;
		HoverVelocity += HoverAcceleration;
		Rotation.Pitch = 0.0f;
	}

	// Interp angle to match the terrain
	Rotation = UKismetMathLibrary::RInterpTo( GetActorRotation(), Rotation, DeltaTime, FloorRotateInterpSpeed );
	SetActorRotation( Rotation );
}

void ABasePlayerPawn::SetupPlayerInputComponent( class UInputComponent* InputCmp )
{
	Super::SetupPlayerInputComponent( InputCmp );

	InputCmp->BindAxis( "MoveSideways", this, &ABasePlayerPawn::MoveSidewaysInput );
	InputCmp->BindVectorAxis( "Gravity", this, &ABasePlayerPawn::Gravity );
}

void ABasePlayerPawn::MoveSideways( float AxisValue )
{
	if( !DisableMovement )
	{
		AxisValue = FMath::Clamp( AxisValue, -1.0f, 1.0f );

		if( CurrentUpgrade == EUpgradeType::EUT_INVERT_CONTROLS )
			AxisValue *= -1.0f;

		if( IsAlive )
		{
			auto Increase = StrafeAcceleration * AxisValue * GetWorld()->DeltaTimeSeconds * ( IsValid( CurrentTurnFloorPiece ) ? 0.6f : 1.0f );
			StrafeVelocity = FMath::Clamp( StrafeVelocity + Increase, -StrafeMaxSpeed, StrafeMaxSpeed );
		}
	}
}

void ABasePlayerPawn::MoveSidewaysInput( float AxisValue )
{
	if( InputMode == EInputMode::EIM_KEYBOARD )
	{
		if( AxisValue )
		{
			CurrentRoll = AxisValue;
			MoveSideways( AxisValue );
		}
		else
			CurrentRoll = 0.0f;
	}
}

void ABasePlayerPawn::SetInputMode( EInputMode NewInputMode )
{
	UCubeSingletonDataLibrary::CustomLog( "Input Mode = " + FString::FromInt( ( int32 )NewInputMode ) );
	InputMode = NewInputMode;
}

void ABasePlayerPawn::MoveSidewaysInputButtons( float AxisValue )
{
	if( InputMode == EInputMode::EIM_SCREEN_BUTTONS )
	{
		if( AxisValue )
		{
			CurrentRoll = AxisValue;
			MoveSideways( AxisValue );
		}
		else
			CurrentRoll = 0.0f;
	}
}

void ABasePlayerPawn::Gravity( FVector Gravity )
{
	if( InputMode == EInputMode::EIM_GYROSCOPIC && !DisableMovement )
	{
		if( Gravity.Size() != 0.0f )
		{
			const auto NormalisedAngle = FMath::Min( 1.0f, FMath::Max( -1.0f, Gravity.Y / 4.0f ) );
			CurrentRoll = NormalisedAngle * RollMax;
			MoveSideways( FMath::Min( 1.0f, FMath::Max( -1.0f, NormalisedAngle * RotationRateSensitivity ) ) );
			auto NewRotation = GetActorRotation();
			NewRotation.Roll = CurrentRoll;
			SetActorRotation( NewRotation );
		}
	}
}

void ABasePlayerPawn::ApplyFriction()
{
	StrafeVelocity *= StrafeFriction;

	if( FMath::Abs( StrafeVelocity ) <= 0.1f )
		StrafeVelocity = 0.0f;

	HoverVelocity *= HoverFriction;

	if( FMath::Abs( HoverVelocity ) <= 0.1f )
		HoverVelocity = 0.0f;
}

void ABasePlayerPawn::BeginTurn( ABaseTurnFloorPiece* TurnPiece )
{
	UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::BeginTurn: " + TurnPiece->GetName() );
	TurnPiece->BeginTurn( GetActorLocation(), ForwardSpeed );
	CurrentTurnFloorPiece = TurnPiece;
}

void ABasePlayerPawn::Explode( bool force /*= false*/ )
{
	if( force || bCanBeDamaged )
	{	
		// Can only lose once & cannot lose after you have won!
		if( IsAlive )
			Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) )->GameEnd( EGameEndState::EGES_FAIL );

		if( const auto Particle = UCubeSingletonDataLibrary::GetGameData()->ExplosionParticle )
			UGameplayStatics::SpawnEmitterAtLocation( GetWorld(), Particle, Mesh->GetComponentLocation() )->SetWorldScale3D( FVector( ExplosionSize ) );

		IsAlive = false;
		Mesh->SetVisibility( false );
		DisableMovement = true;
	}
}

void ABasePlayerPawn::AddUpgrade( ABaseUpgrade* Upgrade )
{
	UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::AddUpgrade | Adding Upgrade: " + FString::FromInt( static_cast< int32 >( Upgrade->UpgradeType ) ), LogDisplayType::Gameplay );

	// Destroy upgrade & spawn particle
	if ( const auto Particle = UCubeSingletonDataLibrary::GetGameData()->UpgradeParticle )
		UGameplayStatics::SpawnEmitterAtLocation( GetWorld(), Particle, GetActorLocation( ) )->SetWorldScale3D( FVector( 5.0f ) );

	auto* GameMode = Cast< ACubeRunnerGameMode >( UGameplayStatics::GetGameMode( GetWorld() ) );
	
	if( IsValid( GameMode ) )
		GameMode->OnUpgradeAdded( Upgrade );

	CurrentUpgrade = Upgrade->UpgradeType;
	UpgradeTimer = 8.0f;

	switch( Upgrade->UpgradeType )
	{
		case EUpgradeType::EUT_SPEED_SLOW:
		{
			PreviousForwardSpeed = ForwardSpeed;
			ForwardSpeed *= 0.75f;
			break;
		}
		case EUpgradeType::EUT_SPEED_FAST:
		{
			PreviousForwardSpeed = ForwardSpeed;
			ForwardSpeed *= 1.25f;
			break;
		}
		default:
		{
			UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::AddUpgrade | Type not handled: " + FString::FromInt( static_cast< int32 >( Upgrade->UpgradeType ) ), LogDisplayType::Warn );
			break;
		}
	}

	//Upgrade->Destroy();
}

void ABasePlayerPawn::RemoveUpgrade( EUpgradeType Type )
{
	switch( Type )
	{
		case EUpgradeType::EUT_SPEED_SLOW:
			// Fall through
		case EUpgradeType::EUT_SPEED_FAST:
		{
			ForwardSpeed = PreviousForwardSpeed;
			PreviousForwardSpeed = 0.0f;
			break;
		}
		default:
		{
			UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::RemoveUpgrade | Type not handled: " + FString::FromInt( static_cast< int32 >( Type ) ), LogDisplayType::Warn );
			break;
		}
	}
}

void ABasePlayerPawn::OnMeshOverlapBegin( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult )
{
	auto CubeGM = Cast< ACubeRunnerGameMode >( GetWorld()->GetAuthGameMode() );
	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );

	// Obstacles (outside of IsAlive check because when you complete a level, the pawn keeps moving but IsAlive is false)
	if( IsValid( Cast< ABaseObstacle >( OtherActor ) ) )
		Explode();

	if( IsValid( Cast< UBaseObstacleComponent >( OtherComp ) ) )
		Explode();

	if( IsValid( Cast< UInstancedStaticMeshComponent >( OtherComp ) ) )
		Explode();

	UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::OverlapBegin: " + OtherActor->GetName() + ( OtherComp ? " : " + OtherComp->GetName() : "" ) );

	if( IsAlive )
	{
		// Floor piece
		if( auto* FloorPiece = Cast< ABaseFloorPiece >( OtherActor ) )
		{
			// Turn pieces
			if( auto* TurnPiece = Cast< ABaseTurnFloorPiece >( OtherActor ) )
			{
				if( TurnPiece->TurnZone == OtherComp )
				{
					if( CurrentTurnFloorPiece )
					{
						const auto Yaw = UKismetMathLibrary::MakeRotFromX( CurrentTurnFloorPiece->TurnEndPoint->GetForwardVector() ).Yaw;
						SetActorRotation( FRotator( GetActorRotation().Pitch, Yaw, GetActorRotation().Roll ) );
					}

					BeginTurn( TurnPiece );
				}
			}

			// Collide
			if( Cast< UBoxComponent >( OtherComp ) )
			{
				// Multi collision
				if( FloorPiece->MultiConnections.Num() > 1 )
				{
					for( int32 i = 0; i < FloorPiece->MultiConnections.Num(); ++i )
					{
						if( OtherComp == FloorPiece->MultiConnections[ i ].Collider )
						{
							CubeGM->SpawnQueueRoot = ( CubeGM->SpawnQueueRoot->NextQueueItems.Num() ? CubeGM->SpawnQueueRoot->NextQueueItems[i] : nullptr );
							CubeGM->SpawnFloorPiece( FloorPiece->MultiConnections[i].ConnectionPoint->GetComponentTransform() );
							break;
						}
					}
				}
				// Floor piece collision
				else if( FloorPiece->SpawnCollison == OtherComp && !FloorPiece->HasTriggered )
				{
					// Handle end collision
					if( FloorPiece->EndLevelPiece )
					{
						UCubeSingletonDataLibrary::CustomLog( "Level Complete" );
						CubeGM->GameEnd( EGameEndState::EGES_SUCCESS );

						if( !CubeGM->LevelSpawnAfterFinish )
							return;
					}

					// Only spawn / remove if not a transition piece
					if( !IsValid( Cast< ABaseTransitionFloorPiece >( FloorPiece ) ) || ( !CubeGM->LevelSpawnTransitions && HasFirstCollision ) )
					{
						if( CubeGM->PreSpawnedPieces >= 1 )
						{
							CubeGM->PreSpawnedPieces--;
							UCubeSingletonDataLibrary::CustomLog( "ACubeRunnerGameMode | PreSpawnedPieces: " + FString::FromInt( CubeGM->PreSpawnedPieces ) );
							return;
						}

						CubeGM->SpawnFloorPiece( FloorPiece->ConnectionPoint->GetComponentTransform() );
						CubeGM->RemoveFloorPiece();
					}

					FloorPiece->HasTriggered = true;
					HasFirstCollision = true;
				}
			}
		}

		// Upgrades
		else if( auto* Upgrade = Cast< ABaseUpgrade >( OtherActor ) )
		{
			AddUpgrade( Upgrade );
		}

		// Wall piece
		else if( auto* WallPiece = Cast< ABaseWallPiece >( OtherActor ) )
		{
			CurrentWallPiece = WallPiece;
			const auto Direction = CurrentWallPiece->GetActorLocation() - GetActorLocation();
			const float Dist = FVector::DotProduct( Direction, GetActorRightVector() );
			CurrentWallPiece->LeftWall = Dist <= 0.0f;
		}
		
		// If no matches, just check if we have directly hit into the actor (which results in the pawn exploding)
		else if( IsValid( Cast< UStaticMeshComponent >( OtherComp ) ) )
		{
			const float dot = FVector::DotProduct( SweepResult.ImpactNormal, GetActorForwardVector() );
			const float angle = acos( dot );
			const float threshold = FMath::RadiansToDegrees( 5.0f );

			// Angle between obstacle is almost directly facing opposite
			if( angle >= ( 2.0f * PI ) - threshold )
			{
				Explode();
			}
			else if( angle >= PI - threshold && angle <= PI + threshold )
			{
			//	CurrentWallPiece = OtherActor;
			}
		}
	}
}

void ABasePlayerPawn::OnMeshOverlapEnd( class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex )
{
	//UCubeSingletonDataLibrary::CustomLog( "ABasePlayerPawn::OverlapEnd" );
	if( auto* turn_piece = Cast< ABaseTurnFloorPiece >( OtherActor ) )
	{
		if( turn_piece->TurnZone == OtherComp && CurrentTurnFloorPiece && CurrentTurnFloorPiece == turn_piece )
		{
			const auto Yaw = UKismetMathLibrary::MakeRotFromX( CurrentTurnFloorPiece->TurnEndPoint->GetForwardVector() ).Yaw;
			SetActorRotation( FRotator( GetActorRotation().Pitch, Yaw, GetActorRotation().Roll ) );
			CurrentTurnFloorPiece = nullptr;
		}
	}
	else if( auto* wall_piece = Cast< ABaseWallPiece >( OtherActor ) )
	{
		CurrentWallPiece = nullptr;
	}
}

void ABasePlayerPawn::IncreaseSpeed( const int32 Speed, const float DeltaTime /*= 1.0f*/ )
{
	ForwardSpeed += Speed * DeltaTime;
	PreviousForwardSpeed += Speed * DeltaTime;
	UpdateStrafeMaxSpeed();
}

void ABasePlayerPawn::SetSpeed( const int32 Speed )
{
	ForwardSpeed = Speed;
	UpdateStrafeMaxSpeed();
}

void ABasePlayerPawn::UpdateStrafeMaxSpeed()
{
	const float Percent = FMath::Max( 1.0f, 0.9f * ( ForwardSpeed / ForwardBaseSpeed ) );
	StrafeMaxSpeed = StrafeBaseMaxSpeed * Percent;
	StrafeAcceleration = StrafeBaseAcceleration * Percent;
}
