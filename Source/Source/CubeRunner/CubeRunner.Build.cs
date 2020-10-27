// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class CubeRunner : ModuleRules
{
    public CubeRunner( ReadOnlyTargetRules Target ) : base( Target )
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem" } );

        DynamicallyLoadedModuleNames.Add( "OnlineSubsystemNull" );

        if( Target.Platform == UnrealTargetPlatform.IOS )
        {
            PrivateDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "OnlineSubsystem" } );
            DynamicallyLoadedModuleNames.Add( "IOSAdvertising" );
            //DynamicallyLoadedModuleNames.Add( "OnlineSubsystemIOS" );
            //DynamicallyLoadedModuleNames.Add( "OnlineSubsystemFacebook" );
        }
        else if( Target.Platform == UnrealTargetPlatform.Android )
        {
            PrivateDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "OnlineSubsystem" } );
            DynamicallyLoadedModuleNames.Add( "AndroidAdvertising" );
            //DynamicallyLoadedModuleNames.Add( "OnlineSubsystemGooglePlay" );
        }
    }
}
