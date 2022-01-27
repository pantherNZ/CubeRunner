// Fill out your copyright notice in the Description page of Project Settings.

#include "CubeRunnerGameMode.h"
#include "CubeRunner.h"
#include "BasePlayerPawn.h"
#include "BaseRandomisedFloorPiece.h"
#include "BaseTransitionFloorPiece.h"
#include "BaseFloorPiece.h"
#include "CubeSaveGame.h"
#include "CubeGameInstance.h"
#include "EndLevelPawn.h"
#include "CubeDataSingleton.h"

#include <functional>
#include <random>
#include "CubeSingletonDataLibrary.h"

ACubeRunnerGameMode::ACubeRunnerGameMode( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
	, PlayerRef( nullptr )
	, EndlessMode( true )
	, FloorPieceOverride( nullptr )
	, RemovalDelay( 1 )
	, GameProgress( 1.0f )
	, GameProgressPerMinute( 4.0f )
	, ClassicPawnClass( nullptr )
	, AdvancedPawnClass( nullptr )
	, LevelFogOpacity( 1.0f )
	, LevelRandomisedFloorPieceDensity( 400 )
	, DistanceMoved( 0.0f )
	, UpdateNewFloorPiecePosition( false )
	, RepeatCount( 1 )
	, PreSpawnedPieces( -1 )
	, ClassicMode( true )
	, PrivateFamily( EPieceFamily::EPF_RANDOMISED )
	, PrivateLengthRemaining( 0 )
	, SpawnQueueRoot( nullptr )
	, SpawnQueueCurrent( nullptr )
	, LevelOptionsSet( false )
	, LevelPreSpawningEnabled( true )
	, LevelPreSpawningCount( 0 )
	, LevelSpawnTransitions( true )
	, LevelPlayerStartSpeed( 800.0f )
	, LevelPlayerAcceleration( 15.0f )
	, LevelSpawnAfterFinish( true )
{

}

void ACubeRunnerGameMode::BeginPlay()
{
	auto* DataSingleton = UCubeSingletonDataLibrary::GetSingletonGameData();

	// Menu
	if( !ClassicPawnClass || !AdvancedPawnClass )
	{
		RegisterFloorPieceFamilies();
		RegisterFloorPieceClasses();
	}
	else
	{
		DataSingleton->CorrectUpgradeSpawnOdds();

		// Initial player spawn etc..
		auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
		ClassicMode = GameInstance->ClassicPlayerMode;
		const auto LevelIndex = GameInstance->LevelIndex;
		EndlessMode = LevelIndex == -1;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = FName( TEXT( "Player" ) );
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		FTransform Transform;

		// Spawn, possess, start timer
		PlayerRef = Cast< ABasePlayerPawn >( GetWorld()->SpawnActor( ClassicMode ? ClassicPawnClass : AdvancedPawnClass, &Transform, SpawnParams ) );
		Transform.SetLocation( FVector( 400.0f, 0.0f, PlayerRef->HoverHeight ) );
		PlayerRef->SetActorTransform( Transform );

		if( !PlayerRef )
		{
			UCubeSingletonDataLibrary::CustomLog( "Player Pawn failed to spawn!", LogDisplayType::Error );
			return;
		}

		UGameplayStatics::GetPlayerController( GetWorld(), 0 )->Possess( PlayerRef );
		PlayerRef->StartTimer = 5.0f;

		// Load game options
		if( !GameInstance->LoadCustomValue( "PlayerTiltSensitivity", PlayerRef->RotationRateSensitivity ) )
			UCubeSingletonDataLibrary::CustomLog( "RotationRateSensitivity value failed to load!", LogDisplayType::Error );

		if( UGameplayStatics::GetPlatformName() == "Android" || UGameplayStatics::GetPlatformName() == "IOS" )
		{
			bool MobileRotation = false;
			if( !GameInstance->LoadCustomBool( "MobileRotationInput", MobileRotation ) )
				UCubeSingletonDataLibrary::CustomLog( "RotationRateSensitivity value failed to load!", LogDisplayType::Error );

			PlayerRef->SetInputMode( MobileRotation ? EInputMode::EIM_GYROSCOPIC : EInputMode::EIM_SCREEN_BUTTONS );
		}
		else
			PlayerRef->SetInputMode( EInputMode::EIM_KEYBOARD );

		// Registry	
		RegisterFloorPieceFamilies();
		RegisterFloorPieceClasses();

		// Load levels
		if( !EndlessMode )
		{
			UCubeSingletonDataLibrary::CustomLog( "Level selected: " + FString::FromInt( LevelIndex ), LogDisplayType::Gameplay );
			ClassicMode ? LoadClassicLevel( LevelIndex ) : LoadAdvancedLevel( LevelIndex );
		}
		else 
			UCubeSingletonDataLibrary::CustomLog( "Endless Mode Started", LogDisplayType::Gameplay );
	}

	// BP BeginPlay
	ReceiveBeginPlay();

	// Load level options
	if( LevelOptionsSet )
	{
		PlayerRef->SetSpeed( LevelPlayerStartSpeed );
		PlayerRef->ForwardSpeedIncrease = LevelPlayerAcceleration;

		if( LevelPlayerStartTransform.GetLocation() != FVector( 0.0f, 0.0f, 0.0f ) )
			PlayerRef->SetActorTransform( LevelPlayerStartTransform );

		PlayerRef->AddActorWorldOffset( LevelStartLocation );
	}

	// First piece is always a randomised cube field
	if( EndlessMode )
	{
		SpawnQueueCurrent = nullptr;
		SpawnFloorPiece( FTransform( ), UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass );
		
		// Normalise probabilities
		TArray< float > TotalWeights;
		TotalWeights.SetNum( 10 );

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			FloorPiece.Difficulty = FMath::Min( 10, FMath::Max( 0, FloorPiece.Difficulty ) );
			TotalWeights[ FloorPiece.Difficulty ] += FloorPiece.Probability;
		}

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			FloorPiece.Probability *= 1.0f / TotalWeights[ FloorPiece.Difficulty ];
		}

		// This sorts the array by difficulty (by overriding the FFloorPieceType struct < operator)
		FloorPieceBPClasses.Sort();
	}
	else
	{
		// Queue the end piece
		SpawnQueueCurrent = nullptr;

		// Pre spawn pieces if required
		while( SpawnQueueRoot )
		{
			FTransform Transform;

			if( FloorPieceArray.Num() )
				Transform = FloorPieceArray.Last()->ConnectionPoint->GetComponentTransform();
			else
				Transform.SetLocation( LevelStartLocation );

			SpawnFloorPiece( Transform );
			
			if( !IsValid( Cast< ABaseTransitionFloorPiece >( FloorPieceArray.Last() ) ) )
				PreSpawnedPieces++;

			if( !SpawnQueueRoot || SpawnQueueRoot->PieceClass == UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass )
				break;

			if( !LevelPreSpawningEnabled || ( LevelPreSpawningCount && PreSpawnedPieces >= ( LevelPreSpawningCount - 1 ) ) )
				break;
		}
	}
}

void ACubeRunnerGameMode::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	// Advance the game (slowly spawns higher difficulty pieces) - limit to 10 difficulty
	GameProgress = FMath::Min( 10.0f, GameProgress + DeltaTime * GameProgressPerMinute / 60.0f );

	// This handles updating the new floor peice position so that it stays within a certain range of the player (sideways movement)
	// Used for randomised cube field floor piece where there is infinite sideways movement
	if( UpdateNewFloorPiecePosition && IsValid( PlayerRef ) )
	{
		if( FloorPieceArray.Num() >= 2 )
		{
			auto* FloorPiece1 = Cast< ABaseRandomisedFloorPiece >( FloorPieceArray[ FloorPieceArray.Num() - 1 ] );
			auto* FloorPiece2 = Cast< ABaseRandomisedFloorPiece >( FloorPieceArray[ FloorPieceArray.Num() - 2 ] );

			if( !IsValid( FloorPiece1 ) || !IsValid( FloorPiece2 ) )
			{
				UpdateNewFloorPiecePosition = false;
				return;
			}

			auto AlignedRightVector = FVector( PlayerRef->GetActorRightVector().X, PlayerRef->GetActorRightVector().Y, 0.0f );
			auto Dist = FVector::DotProduct( ( PlayerRef->GetActorLocation() - FloorPiece2->FloorMesh->GetComponentLocation() ), AlignedRightVector );

			auto Pos = FloorPiece2->FloorMesh->GetComponentLocation();
			auto XOffset = FloorPiece2->GetActorForwardVector() * FloorPiece2->FloorMesh->GetRelativeLocation().X * 2.0f;
			auto RightOffset = AlignedRightVector * Dist;

			FloorPiece1->MoveFloor( Pos + RightOffset + XOffset, RightOffset.Size() * FMath::Sign( Dist ) );
		}
	}
}

FVector ACubeRunnerGameMode::LocationRounded( const FVector& Loc )
{
	return FVector( FMath::RoundToInt( Loc.X ), FMath::RoundToInt( Loc.Y ), FMath::RoundToInt( Loc.Z ) );
}

void ACubeRunnerGameMode::SpawnFloorPiece( const FTransform& Transform, UClass* PieceOverride /*= nullptr*/, int32 VariationOverride /*= 0*/ )
{
	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );

	// Deciding which piece type to spawn
	UClass* FloorPieceType = nullptr;
	
	if( PieceOverride != nullptr )
	{
		FloorPieceType = PieceOverride;
		SpawnFloorPieceInternal( FloorPieceType, VariationOverride, false, Transform, true );
	}
	else if( FloorPieceOverride != nullptr )
	{
		FloorPieceType = FloorPieceOverride;
		SpawnFloorPieceInternal( FloorPieceType, VariationOverride, false, Transform, true );
	}
	else
	{
		// Find piece pseudo randomly
		if( !SpawnQueueRoot )
			FindFloorPieceToSpawn();

		// Pop from front of queue and use that
		auto* CurrentQueue = SpawnQueueRoot;
		FloorPieceType = CurrentQueue->PieceClass;

		// Spawn
		auto* NewPiece = SpawnFloorPieceInternal( FloorPieceType, VariationOverride == 0 ? CurrentQueue->Variation : VariationOverride, false, Transform, true );

		// Spawn all extra connections pieces now (split piece)
		if( CurrentQueue->NextQueueItems.Num() > 1 )
		{
			//SpawnExtraConnections( NewPiece );
			return;
		}

		// We only move along the queue if this isn't a multi piece (this is because we don't know which path down the multi piece we are going yet)
		SpawnQueueRoot = ( CurrentQueue->NextQueueItems.Num() ? CurrentQueue->NextQueueItems[ 0 ] : nullptr );
		delete CurrentQueue;
	}

	UCubeSingletonDataLibrary::CustomLog( "Spawning Piece: " + FloorPieceType->GetName() + " with variation: " + ( VariationOverride == -1 ? " Random" : FString::FromInt( VariationOverride ) ) );
	
	// Should the piece be updated
	if( ( FloorPieceType == UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass ) &&
		( FloorPieceArray.Num() > 1 && FloorPieceArray[ FloorPieceArray.Num() - 2 ]->GetClass() == UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass ) )
	{
		UpdateNewFloorPiecePosition = true;
	}
}

ABaseFloorPiece* ACubeRunnerGameMode::SpawnFloorPieceInternal( UClass* PieceClass, int32 PieceVariation, bool TransitionPiece, const FTransform& Transform, const bool AddToArray )
{
	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );

	//-----------------------------------------------------------
	// Spawn the main floor piece
	FTransform PieceTransform = Transform;
	PieceTransform.SetScale3D( FVector( 1.0f, 1.0f, 1.0f ) );
	PieceTransform.SetLocation( LocationRounded( Transform.GetLocation() ) );

	//auto NewPiece = Cast< ABaseFloorPiece >( GetWorld()->SpawnActor( PieceClass, &PieceTransform ) );	
	// Deffer so we can set the variation BEFORE the construction script gets called for the piece
	auto NewPiece = Cast< ABaseFloorPiece >( UGameplayStatics::BeginDeferredActorSpawnFromClass( this, PieceClass, PieceTransform, ESpawnActorCollisionHandlingMethod::AlwaysSpawn ) );
	//NewPiece->SetActorTransform( PieceTransform );
	if( !IsValid( NewPiece ) )
	{
		UCubeSingletonDataLibrary::CustomLog( "Spawning floor piece failed! Class: " + ( PieceClass != nullptr ? PieceClass->GetName() : "nullptr" ), LogDisplayType::Error );
		return nullptr;
	}

	//NewPiece->AddActorLocalOffset( -NewPiece->RootDir->GetRelativeTransform().GetLocation() );
	//PieceTransform.AddToTranslation( -NewPiece->RootDir->GetRelativeTransform().GetLocation() );
	NewPiece->Variation = PieceVariation == -1 ? FMath::RandRange( 0, ClassicMode ? NewPiece->MaxVariationClassic : NewPiece->MaxVariationAdvanced ) : PieceVariation;
	UGameplayStatics::FinishSpawningActor( NewPiece, PieceTransform );
	NewPiece->SetActorTransform( PieceTransform );
	//--------------------------------------------------------------

	// Decrement the counter for private length (for "family" pieces) 
	if( !TransitionPiece )
		PrivateLengthRemaining = FMath::Max( PrivateLengthRemaining - 1, 0 );

	// Spawn transition pieces
	// -------------------------------------------------------------
	if( !TransitionPiece && !NewPiece->EndLevelPiece && LevelSpawnTransitions )
	{
		// Try spawning the end transition piece for the previous piece
		// This happens if we are changing "family" EG. Outdoor piece into indoor piece.
		if( FloorPieceArray.Num() > 0 )
		{
			const auto PreviousFamily = FloorPieceArray.Last()->PieceFamily;
			const auto DifferingFamily = PreviousFamily != NewPiece->PieceFamily;
			const auto PreviousPieceData = PieceFamilyData[ PreviousFamily ];

			// End transition piece
			if( PreviousPieceData.EndTransitionPiece != nullptr && DifferingFamily )
			{
				// Spawn it
				SpawnFloorPieceInternal( PreviousPieceData.EndTransitionPiece, 0, true, FloorPieceArray.Last()->ConnectionPoint->GetComponentTransform(), true );
				UCubeSingletonDataLibrary::CustomLog( "Spawning end transition piece: " + PreviousPieceData.EndTransitionPiece->GetName() );

				// Update the transform of the new main piece (needs to be "pushed forward")
				FTransform UpdatedPieceTransform = FloorPieceArray.Last()->ConnectionPoint->GetComponentTransform();
				UpdatedPieceTransform.SetScale3D( FVector( 1.0f, 1.0f, 1.0f ) );
				UpdatedPieceTransform.SetLocation( LocationRounded( UpdatedPieceTransform.GetLocation() ) );
				NewPiece->SetActorTransform( UpdatedPieceTransform );
			}
		}

		// New piece data
		const auto NewPieceData = PieceFamilyData.Find( NewPiece->PieceFamily );

		if( !NewPieceData )
		{
			UCubeSingletonDataLibrary::CustomLog( "Failed to find piece family data with floor piece: " + NewPiece->GetPathName(), LogDisplayType::Error );
		}
		else
		{
			// Start Transition piece
			// Again, happens if we are "differing family" from previous piece OR if this is the first piece
			if ( NewPieceData->StartTransitionPiece != nullptr &&
				( ( FloorPieceArray.Num() > 0 && FloorPieceArray.Last()->PieceFamily != NewPiece->PieceFamily ) ||
					FloorPieceArray.Num() == 0 ) )
			{
				FTransform NewTransform;
				NewTransform.SetLocation( LevelStartLocation );

				// Must check this because it can be spawned as the first piece (which will use a default transform)
				if ( FloorPieceArray.Num() )
					NewTransform = FloorPieceArray.Last()->ConnectionPoint->GetComponentTransform();

				// Spawn it
				SpawnFloorPieceInternal( NewPieceData->StartTransitionPiece, 0, true, NewTransform, true );
				UCubeSingletonDataLibrary::CustomLog( "Spawning start transition piece: " + NewPieceData->StartTransitionPiece->GetName() );

				// Update the main floor piece with new transform (changed because of the spawned start transition piece)
				FTransform UpdatedPieceTransform = FloorPieceArray.Last()->ConnectionPoint->GetComponentTransform();
				UpdatedPieceTransform.SetScale3D( FVector( 1.0f, 1.0f, 1.0f ) );
				UpdatedPieceTransform.SetLocation( LocationRounded( UpdatedPieceTransform.GetLocation() ) );
				NewPiece->SetActorTransform( UpdatedPieceTransform );

				// Private family run
				if ( NewPieceData->PrivateFamily )
				{
					PrivateFamily = NewPiece->PieceFamily;
					PrivateLengthRemaining = FMath::Max( FMath::RandRange( NewPieceData->PrivateFamilyLengthMin, NewPieceData->PrivateFamilyLengthMax ) - 1, 0 );
				}
			}
		}
	}
	//--------------------------------------------------------------


	// Notify other pieces of this spawn (for cooldown handling)
	if( !TransitionPiece )
	{
		for( auto Piece : FloorPieceArray )
			Piece->NewFloorPieceSpawned( NewPiece );
	}

	// Custom begin play 
	Cast< ABaseFloorPiece >( NewPiece )->FloorPieceBeginPlay();

	// Add the piece to the spawned pieces array
	if( AddToArray )
		FloorPieceArray.Add( NewPiece );

	return NewPiece;
}

void ACubeRunnerGameMode::SpawnExtraConnections( ABaseFloorPiece* BaseMultiPiece )
{
	for( int32 i = 0; i < SpawnQueueRoot->NextQueueItems.Num(); ++i )
	{
		if( i > BaseMultiPiece->MultiConnections.Num() )
		{
			UCubeSingletonDataLibrary::CustomLog( "Spawning Multi Piece item: Not enough MultiConnection Members" );
			break;
		}

		const auto& item = *SpawnQueueRoot->NextQueueItems[i];
		auto* ExtraPiece = SpawnFloorPieceInternal( item.PieceClass, item.Variation, false, BaseMultiPiece->MultiConnections[i].ConnectionPoint->GetComponentTransform(), false );
		BaseMultiPiece->MultiConnections[i].ConnectedSpawnPiece = ExtraPiece;
	}
}

void ACubeRunnerGameMode::MultiPieceCollision( ABaseFloorPiece* BaseMultiPiece, const int32 index )
{
	if( !SpawnQueueRoot )
	{
		UCubeSingletonDataLibrary::CustomLog( "CheckMultiPieceCollision: SpawnQueueRoot is not valid", LogDisplayType::Error );
		return;
	}

	// Move the queue forwards down the correct multi piece path
	auto* CurrentQueue = SpawnQueueRoot;
	SpawnQueueRoot = ( index < SpawnQueueRoot->NextQueueItems.Num() ? SpawnQueueRoot->NextQueueItems[ index ] : nullptr );

	const bool NextIsMulti = SpawnQueueRoot && SpawnQueueRoot->NextQueueItems.Num() > 1;
	SpawnQueueRoot = ( NextIsMulti ? SpawnQueueRoot : SpawnQueueRoot->NextQueueItems.Num() ? SpawnQueueRoot->NextQueueItems.Last() : nullptr );

	// We must delete the other possible paths now before they are lost
	for( int32 j = 0; j < CurrentQueue->NextQueueItems.Num(); ++j )
	{
		if( j != index )
		{
			CurrentQueue->NextQueueItems[ j ]->PropagateDeletionToChildren = true;
			delete CurrentQueue->NextQueueItems[ j ];
			CurrentQueue->NextQueueItems[ j ] = nullptr;
		}
	}

	// Delete the piece we have moved on from
	delete CurrentQueue;

	if( !SpawnQueueRoot )
		UCubeSingletonDataLibrary::CustomLog( "Failed to find Multi Piece Queue info at index: " + FString::FromInt( index ), LogDisplayType::Error );

	RemoveFloorPiece();

	if( NextIsMulti )
		SpawnExtraConnections( BaseMultiPiece );
}

void ACubeRunnerGameMode::FindFloorPieceToSpawn( const bool IgnoreSplitPieces /*= false*/ )
{
	const auto* DataSingleton = Cast<UCubeDataSingleton>( GEngine->GameSingleton );

	if( ( ( RepeatCount <= 0 ) || 
		( RepeatCount == 1 && FMath::RandRange( 0, 1 ) == 0 ) ||
		( RepeatCount == 2 && FMath::RandRange( 0, 9 ) == 0 ) ) 
		&& PrivateLengthRemaining == 0 )
	{
		RepeatCount++;
		const int32 count = FMath::RandRange( 1, int32( GameProgress / 2 ) );
		for( int32 i = 0; i < count; ++i )
			QueuePiece( UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass );
		return;
	}

	TArray< FFloorPieceType > ValidFloorPieceBPClasses;

	for( auto& PieceType : FloorPieceBPClasses )
	{
		if( IgnoreSplitPieces && PieceType.Connections > 1 )
			continue;

		if( PrivateLengthRemaining && PieceType.Family != PrivateFamily )
			continue;

		// Reverse iterate to find the most recent instance of this floor piece type (if one exists currently)
		const int32 FoundIndex = FloorPieceArray.FindLastByPredicate( [ &PieceType ]( ABaseFloorPiece* Piece )
		{
			return PieceType.FloorPieceBPClass->StaticClass() == Piece->StaticClass();
		} );

		// Checks to make sure this piece can be spawned
		if( FoundIndex == INDEX_NONE || FloorPieceArray[ FoundIndex ]->IsReadyToBePlaced() )
		{
			ValidFloorPieceBPClasses.Add( PieceType );
		}
	}

	if( !ValidFloorPieceBPClasses.Num() )
	{
		RepeatCount++;
		QueuePiece( UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass );
		UCubeSingletonDataLibrary::CustomLog( "No valid floor peices found:" );
		return;
	}
	
	UCubeSingletonDataLibrary::CustomLog( "Valid floor pieces found: " + FString::FromInt( ValidFloorPieceBPClasses.Num() ) );

	RepeatCount = 0;
	const int32 Min = ValidFloorPieceBPClasses[ 0 ].Difficulty;
	const int32 Max = ValidFloorPieceBPClasses[ ValidFloorPieceBPClasses.Num() - 1 ].Difficulty;
	const int32 Range = Max - Min;

	UCubeSingletonDataLibrary::CustomLog( "Min Difficulty: " + FString::FromInt( Min ) );
	UCubeSingletonDataLibrary::CustomLog( "Max Difficulty: " + FString::FromInt( Max ) );
	UCubeSingletonDataLibrary::CustomLog( "Progress: " + FString::SanitizeFloat( GameProgress ) );

	std::default_random_engine generator;
	std::normal_distribution< double > distribution( double( GameProgress ), double( Range / 3.0 ) );

	int32 Safety = 0;

	// Normal distribution of difficulty to find a piece appropriate	
	while( Safety < 20 )
	{
		int32 DifficultyVal = 1;

		if( !Range )
		{
			DifficultyVal = Min;
		}
		else
		{
			// While loop to handle min / max cut offs
			do
			{
				// Normal distribution around GameProgress with standard deviation of Range / 3
				//DifficultyVal = ( int32 )distribution( generator );

				// Linear distribution from min -> max
				DifficultyVal = Min + FMath::RandHelper( FMath::Min( ( int32 )GameProgress, Range ) );
			} 
			while( DifficultyVal < Min || DifficultyVal > Max );
		}

		UCubeSingletonDataLibrary::CustomLog( "Difficulty selected: " + FString::FromInt( DifficultyVal ) );
		float Random = FMath::FRand();
		int32 Matches = 0;
		int32 MatchIndex = 0;

		// Weighted random selection from pieces that match the selected difficulty
		for( int32 i = 0; i < ValidFloorPieceBPClasses.Num(); ++i )
		{
			if( ValidFloorPieceBPClasses[ i ].Difficulty == DifficultyVal )
			{
				Matches++;
				MatchIndex = i;

				if( Random < ValidFloorPieceBPClasses[ i ].Probability )
				{
					if( ValidFloorPieceBPClasses[ MatchIndex ].Connections <= 1 )
					{
						QueuePieceWithRandomVariation( ValidFloorPieceBPClasses[ i ].FloorPieceBPClass );
					}
					else
					{
						QueuePieceSplitWithRandomVariation( ValidFloorPieceBPClasses[ i ].FloorPieceBPClass );

						// If it is a split piece, we must queue pieces for each direction
						for( int32 j = 1; j < ValidFloorPieceBPClasses[ MatchIndex ].Connections; ++j )
						{
							FindFloorPieceToSpawn( true );
							EndSplitPieceQueue();
						}
					}

					return;
				}
				
				Random -= ValidFloorPieceBPClasses[ i ].Probability;
			}
		}

		// Matches were found but none got selected (can happen if the probabilies don't add up to 1.0)
		if( Matches > 0 )
		{
			UCubeSingletonDataLibrary::CustomLog( "FindFloorPieceToSpawn: Matches found but none selected", LogDisplayType::Error );
			QueuePiece( ValidFloorPieceBPClasses[ MatchIndex ].FloorPieceBPClass );
			return;
		}

		// No matches of this difficulty, try again
	}

	// Catch any potential issues
	UCubeSingletonDataLibrary::CustomLog( "FindFloorPieceToSpawn: Max iterations reached (no matches found)", LogDisplayType::Error );
	QueuePiece( UCubeSingletonDataLibrary::GetGameData()->RandomisedFloorPieceBPClass );
}

void ACubeRunnerGameMode::RemoveFloorPiece()
{
	if( FloorPieceArray.Num() > 0 && RemovalDelay == 0 )
	{
		FloorPieceArray[ 0 ]->Cleanup();
		FloorPieceArray[ 0 ]->Destroy();
		FloorPieceArray.RemoveAt( 0 );

		if( FloorPieceArray.Num( ) > 0 && IsValid( Cast< ABaseTransitionFloorPiece >( FloorPieceArray[0] ) ) )
			RemoveFloorPiece();
	}

	RemovalDelay = FMath::Max( RemovalDelay - 1, 0 );
}

void ACubeRunnerGameMode::GameEnd_Implementation( EGameEndState Reason )
{
	// Ensure we are dead!
	PlayerRef->IsAlive = false;

	auto* GameInstance = Cast< UCubeGameInstance >( UGameplayStatics::GetGameInstance( GetWorld() ) );
	if( PlayerRef->TotalDistanceTravelled >= 10000.0f )
		GameInstance->GamesPlayed++;

	if( Reason == EGameEndState::EGES_SUCCESS )
	{
		// Spawn end level pawn (static camera) and possess
		auto* controller = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
		controller->UnPossess();

		FActorSpawnParameters spawn_params;
		spawn_params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const auto transform = PlayerRef->GetTransform();
		auto* end_level_pawn = Cast< AEndLevelPawn >( GetWorld()->SpawnActor( AEndLevelPawn::StaticClass(), &transform, spawn_params ) );
		controller->Possess( end_level_pawn );
		end_level_pawn->SetActorTransform( PlayerRef->Camera->GetComponentTransform() );

		// Pass stats to end level pawn
		end_level_pawn->TotalDistanceTravelled = PlayerRef->TotalDistanceTravelled;

		// Complete level
		GameInstance->CompleteLevel( ClassicMode, GameInstance->LevelIndex );

		// Queue timer for destroy player pawn
		if( IsValid( GetWorld() ) )
		{
			if( !pawn_destroy_handle.IsValid() )
				GetWorld()->GetTimerManager().SetTimer( pawn_destroy_handle, this, &ACubeRunnerGameMode::DestroyPawn, 5.0f );
		}
		else
			UCubeSingletonDataLibrary::CustomLog( "Failed to start timer to destroy pawn after level completed", LogDisplayType::Error );
	}
}

void ACubeRunnerGameMode::DestroyPawn()
{
	PlayerRef->DisableMovement = true;
}

void ACubeRunnerGameMode::LoadMainMenu()
{
	UGameplayStatics::OpenLevel( GetWorld(), TEXT( "Menu" ) );
}

void ACubeRunnerGameMode::TempRestartGame()
{
	FName LevelName( *UGameplayStatics::GetCurrentLevelName( this, true ) );
	UGameplayStatics::OpenLevel( this, LevelName );
	//GetWorld()->GetFirstPlayerController()->ConsoleCommand( TEXT( "RestartLevel" ) );
	UCubeSingletonDataLibrary::CustomLog( "Level restart" );
}

void ACubeRunnerGameMode::QueuePiece( UClass* Class )
{
	QueuePieceMultiWithVariation( Class, 1, 0 );
}

void ACubeRunnerGameMode::QueuePieceWithVariation( UClass* Class, int32 Variation )
{
	QueuePieceMultiWithVariation( Class, 1, Variation );
}

void ACubeRunnerGameMode::QueuePieceWithRandomVariation( UClass* Class )
{
	QueuePieceMultiWithVariation( Class, 1, -1 );
}

void ACubeRunnerGameMode::QueuePieceMulti( UClass* Class, int32 Count )
{
	QueuePieceMultiWithVariation( Class, Count, 0 );
}

void ACubeRunnerGameMode::QueuePieceMultiWithRandomVariation( UClass* Class, int32 Count )
{
	QueuePieceMultiWithVariation( Class, Count, -1 );
}

void ACubeRunnerGameMode::QueuePieceMultiWithVariation( UClass* Class, int32 Count, int32 Variation )
{
	if( Count )
	{
		for( int32 i = 0; i < Count; ++i )
		{		
			FSpawnQueueItem* NewItem = new FSpawnQueueItem;
			NewItem->PieceClass = Class;
			NewItem->Variation = Variation;

			if( !SpawnQueueRoot )
			{			
				SpawnQueueCurrent = NewItem;
				SpawnQueueRoot = NewItem;
			}
			else
			{
				SpawnQueueCurrent->NextQueueItems.Add( NewItem );
				SpawnQueueCurrent = NewItem;
			}
		}
	}
}

void ACubeRunnerGameMode::QueuePieceSplit( UClass* Class )
{
	QueuePieceSplitWithVariation( Class, 0 );
}

void ACubeRunnerGameMode::QueuePieceSplitWithRandomVariation( UClass* Class )
{
	QueuePieceSplitWithVariation( Class, -1 );
}

void ACubeRunnerGameMode::QueuePieceSplitWithVariation( UClass* Class, int32 Variation )
{
	auto* NewItem = new FSpawnQueueItem();
	NewItem->PieceClass = Class;
	NewItem->Variation = Variation;
	NewItem->QueueIndex = 0;

	if( !SpawnQueueRoot )
	{
		SpawnQueueCurrent = NewItem;
		SpawnQueueRoot = NewItem;
	}
	else
	{
		SpawnQueueCurrent->NextQueueItems.Add( NewItem );
		SpawnQueueCurrent = NewItem;
	}

	SpawnQueueSplits.Add( SpawnQueueCurrent );
}

void ACubeRunnerGameMode::EndSplitPieceQueue()
{
	if( SpawnQueueSplits.Num() )
	{
		SpawnQueueSplits.Last()->QueueIndex++;
		SpawnQueueCurrent = SpawnQueueSplits.Last();

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			if( SpawnQueueSplits.Last()->QueueIndex >= FloorPiece.Connections - 1 )
			{
				SpawnQueueSplits.RemoveAt( SpawnQueueSplits.Num() - 1 );
				break;
			}
		}
	}
}

void ACubeRunnerGameMode::SetLevelOptions( bool PreSpawnFloorPieces, float PlayerStartSpeed, float PlayerAcceleration, bool SpawnTransitions, FVector StartLocation, FTransform PlayerStartTransform, bool SpawnAfterFinish )
{
	LevelOptionsSet = true;
	LevelPreSpawningEnabled = PreSpawnFloorPieces;
	LevelPlayerStartSpeed = PlayerStartSpeed;
	LevelPlayerAcceleration = PlayerAcceleration;
	LevelSpawnTransitions = SpawnTransitions;
	LevelStartLocation = StartLocation;
	LevelSpawnAfterFinish = SpawnAfterFinish;
	LevelPlayerStartTransform = PlayerStartTransform;
}

void ACubeRunnerGameMode::SetLevelOptionsWithPreSpawn( int32 FloorPiecesToPreSpawn, float PlayerStartSpeed, float PlayerAcceleration, bool SpawnTransitions, FVector StartLocation, FTransform PlayerStartTransform, bool SpawnAfterFinish )
{
	SetLevelOptions( true, PlayerStartSpeed, PlayerAcceleration, SpawnTransitions, StartLocation, PlayerStartTransform, SpawnAfterFinish );
	LevelPreSpawningCount = FloorPiecesToPreSpawn;
}

bool ACubeRunnerGameMode::CheckValidGameType( ERegistryType Type ) const
{
	return ( Type == ERegistryType::ERT_SHARED ) ||
		( ClassicMode && Type == ERegistryType::ERT_CLASSIC ) ||
		( !ClassicMode && Type == ERegistryType::ERT_ADVANCED );
}

void ACubeRunnerGameMode::RegisterFloorPieceBPClass( UClass* FloorPieceBPClass, int32 Difficulty, float Probability, EPieceFamily Family, int32 Connections /*= 1*/, ERegistryType Type /*= ERT_SHARED*/ )
{
	if( CheckValidGameType( Type ) )
	{
		FFloorPieceType NewPiece;
		NewPiece.FloorPieceBPClass = FloorPieceBPClass;
		NewPiece.Difficulty = Difficulty;
		NewPiece.Probability = Probability;
		NewPiece.Family = Family;
		NewPiece.Connections = Connections;
		FloorPieceBPClasses.Add( NewPiece );
	}
}

void ACubeRunnerGameMode::RegisterFamily( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family, ERegistryType Type /*= ERT_SHARED*/ )
{
	if( CheckValidGameType( Type ) )
	{
		FFloorPieceFamily NewFamily;
		NewFamily.StartTransitionPiece = StartTransitionFloorPiece;
		NewFamily.EndTransitionPiece = EndTransitionFloorPiece;
		NewFamily.Family = Family;
		NewFamily.PrivateFamily = false;
		PieceFamilyData.Add( Family, NewFamily );
	}
}

void ACubeRunnerGameMode::RegisterFamilyWithLength( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family, int32 PrivateFamilyLengthMin, int32 PrivateFamilyLengthMax, ERegistryType Type /*= ERT_SHARED*/ )
{
	if( CheckValidGameType( Type ) )
	{
		RegisterFamily( StartTransitionFloorPiece, EndTransitionFloorPiece, Family );

		PieceFamilyData[ Family ].PrivateFamily = true;
		PieceFamilyData[ Family ].PrivateFamilyLengthMin = PrivateFamilyLengthMin;
		PieceFamilyData[ Family ].PrivateFamilyLengthMax = PrivateFamilyLengthMax;
	}
}

void ACubeRunnerGameMode::RegisterFamilyWithLength2( UClass* StartTransitionFloorPiece, UClass* EndTransitionFloorPiece, EPieceFamily Family, int32 PrivateFamilyLengthMin, int32 PrivateFamilyLengthMax, int32 Difficulty, float Probability, ERegistryType Type /*= ERT_SHARED*/ )
{
	if( CheckValidGameType( Type ) )
	{
		RegisterFamilyWithLength( StartTransitionFloorPiece, EndTransitionFloorPiece, Family, PrivateFamilyLengthMin, PrivateFamilyLengthMax );

		std::vector< float > TotalWeights;
		TotalWeights.resize( 2 );
		uint32 Count = 0;
		int32 OldDifficulty = 0;

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			if( FloorPiece.Family == Family )
			{
				OldDifficulty = FloorPiece.Difficulty;
				break;
			}
		}

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			if( FloorPiece.Family == Family )
			{	
				FloorPiece.Difficulty = Difficulty;
				Count++;
			}
			else
			{
				// Previous difficulty normalise
				if( FloorPiece.Difficulty == OldDifficulty )
					TotalWeights[ 0 ] += FloorPiece.Probability;

				// New difficulty normalise
				if( FloorPiece.Difficulty == Difficulty )
					TotalWeights[ 1 ] += FloorPiece.Probability;
			}
		}

		for( auto& FloorPiece : FloorPieceBPClasses )
		{
			if( FloorPiece.Family == Family )
			{
				FloorPiece.Probability = Probability / Count;
			}
			else
			{
				if( FloorPiece.Difficulty == OldDifficulty )
					FloorPiece.Probability *= 1.0f / TotalWeights[ 0 ];

				if( FloorPiece.Difficulty == Difficulty )
					FloorPiece.Probability *= ( 1.0f - Difficulty ) / TotalWeights[ 1 ];
			}
		}
	}
}
