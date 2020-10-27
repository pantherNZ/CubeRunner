// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "BaseUpgrade.generated.h"

UENUM( BlueprintType )
enum class EUpgradeType : uint8
{
	EUT_SPEED_SLOW UMETA( DisplayName = "Low Speed" ),
	EUT_MINI UMETA( DisplayName = "Mini" ),

	EUT_SPEED_FAST UMETA( DisplayName = "High Speed" ),
	EUT_INVERT_CONTROLS UMETA( DisplayName = "Inverted Controls" ),
	EUT_HEAVY_FOG UMETA( DisplayName = "Restricted Visibility" ),
	EUT_UPSIDE_DOWN UMETA( DisplayName = "Upside Down" ),

	EUT_UPGRADE_MAX_TOTAL UMETA( DisplayName = "Total Upgrade Count" ),
	EUT_UPGRADE_NONE UMETA( DisplayName = "None" ),
	EUT_UPGRADE_POSITIVE_END UMETA( DisplayName = "Total Positive Upgrades" ) = EUT_SPEED_FAST,
};

UCLASS()
class CUBERUNNER_API ABaseUpgrade : public AActor
{
	GENERATED_BODY()

	// Functions
public:
	ABaseUpgrade( const FObjectInitializer& ObjectInitializer );
	virtual void BeginPlay() override;

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	FString GetUpgradeDisplayName();

	// Members
public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere ) EUpgradeType UpgradeType;
};
