// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameInstance.h"
#include "CubeGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FLostFocusSignature );

USTRUCT( BlueprintType )
struct FGameplayStatistics
{
	GENERATED_USTRUCT_BODY()

public:
	FGameplayStatistics() { }
};

USTRUCT( BlueprintType )
struct FAchievement
{
	GENERATED_USTRUCT_BODY()

public:
	FAchievement() { }

	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) FString Name;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float Progress = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float CachedProgress = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) float Goal = 0.0f;
	UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = Data ) UTexture2D* Icon = nullptr;
};

UCLASS()
class CUBERUNNER_API UCubeGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	//Functions:
	UCubeGameInstance( const FObjectInitializer& ObjectInitializer );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	float GetOptionValueScaled( float Value, float Min, float Max );

	UFUNCTION( BlueprintCallable, Category = "Utility" )
	float GetOptionValueRaw( float Value, float Min, float Max );

	// Called when the user logs in or out on a mobile device.
	UFUNCTION( BlueprintImplementableEvent, Category = "Online" )
	void OnLoginChanged( bool bLoggingIn, int32 UserID, int32 UserIndex );

	// Called when the application has just entered the foreground.
	UFUNCTION( BlueprintImplementableEvent, Category = "Online" )
	void OnEnteringForeground();

	// Called when the application is about to enter the background.
	UFUNCTION( BlueprintImplementableEvent, Category = "Online" )
	void OnEnteringBackground();

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void CompleteLevel( const bool ClassicMode, const int32 Level );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	TArray< int32 > LoadCompletedLevels( const bool ClassicMode );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	TArray< int32 > LoadUncompletedLevels( const bool ClassicMode );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	TArray< int32 > LoadCompletedLevelsOfDifficulty( const bool ClassicMode, const int32 Difficulty );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	TArray< int32 > LoadUncompletedLevelsOfDifficulty( const bool ClassicMode, const int32 Difficulty );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void SaveCompletedLevels( const bool ClassicMode, const TArray< int32 >& Levels );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	bool HasUnlockedDifficulty( const TArray< int32 >& Levels, const int32 Difficulty );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	bool LoadCustomValue( FString Key, float& LoadedValue );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	bool LoadCustomBool( FString Key, bool& LoadedValue );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void SaveCustomValue( FString Key, float Value );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void SaveCustomBool( FString Key, bool Value );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void ClearCustomValue( FString Key );

	UFUNCTION( BlueprintCallable, Category = "Save Load System" )
	void SaveGame();

	UFUNCTION( BlueprintCallable, Category = "Online" )
	void RegisterOnlineID( FString NewOnlineID );

protected:
	void Init() override;
	void Shutdown() override;
	void InitSaveGameSlot();
	FString GetSaveSlotName() const;
	bool CheckSaveGame() const;

	// Members
public:
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 LevelIndex = 0;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) bool ClassicPlayerMode = true;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) bool MenuLoadedFromGame = false;

	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 ClassicHighscore = 0.0f;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 AdvancedHighscore = 0.0f;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 GamesPlayed = 0;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) int32 LevelsComplete = 0;
	UPROPERTY( BlueprintReadWrite, EditAnywhere, Category = "Data" ) TArray< FAchievement > AchievementData;

	UPROPERTY( BlueprintAssignable, Category = "Events" ) FLostFocusSignature OnLostFocus;

	UPROPERTY() class UCubeSaveGame* InstanceSaveGameData = nullptr;

protected:	
	FString SaveGamePrefix;
	FString DefaultSaveGameSlot = "_AnOddReflex";

	FDelegateHandle LoginChangedHandle;
	FDelegateHandle EnteringForegroundHandle;
	FDelegateHandle EnteringBackgroundHandle;
};
