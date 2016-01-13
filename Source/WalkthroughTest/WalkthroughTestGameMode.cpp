// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WalkthroughTest.h"
#include "WalkthroughTestGameMode.h"
#include "WalkthroughTestHUD.h"
#include "WalkthroughTestCharacter.h"

AWalkthroughTestGameMode::AWalkthroughTestGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPersonCPP/Blueprints/FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

	// use our custom HUD class
	HUDClass = AWalkthroughTestHUD::StaticClass();
}
