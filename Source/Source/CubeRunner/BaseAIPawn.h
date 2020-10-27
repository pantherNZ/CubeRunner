// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "BasePlayerPawn.h"
#include <vector>
#include "BaseAIPawn.generated.h"

struct PathNode
{
	PathNode* CameFrom;
	FVector Location;
	
	PathNode( FVector Pos ) : Location( Pos ), CameFrom( nullptr ) 
	{ 
	}
};

UCLASS()
class CUBERUNNER_API ABaseAIPawn : public ABasePlayerPawn
{
	GENERATED_BODY()
	
	// Functions
public:
	ABaseAIPawn( const FObjectInitializer& ObjectInitializer );

	virtual void BeginPlay() override;
	virtual void Tick( float DeltaSeconds ) override;
	virtual void SetupPlayerInputComponent( class UInputComponent* InputComponent ) override;

private:
	void ProcessPathFinding( float DeltaSeconds );
	void ReconstructPath( PathNode* FinalNode );
	void ProcessAIMovement( float DeltaSeconds );
	bool CheckLocationCollision( FVector Location );

	// Members
public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) int32 GridSize;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) int32 MinimumPathDistance;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Stats" ) int32 MaximumPathDistance;

private:
	std::vector< FVector > CurrentPath;
};
