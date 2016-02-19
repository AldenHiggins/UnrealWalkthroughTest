// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WalkthroughTest.h"
#include "WalkthroughTestCharacter.h"
#include "WalkthroughTestProjectile.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/InputSettings.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>

DEFINE_LOG_CATEGORY_STATIC(LogFPChar, Warning, All);

//////////////////////////////////////////////////////////////////////////
// AWalkthroughTestCharacter

AWalkthroughTestCharacter::AWalkthroughTestCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->AttachParent = GetCapsuleComponent();
	FirstPersonCameraComponent->RelativeLocation = FVector(0, 0, 64.f); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
}

//////////////////////////////////////////////////////////////////////////
// Input

void AWalkthroughTestCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// set up gameplay key bindings
	check(InputComponent);

	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAction("DPAD_LEFT", IE_Pressed, this, &AWalkthroughTestCharacter::saveLevelToJson);
	InputComponent->BindAction("DPAD_RIGHT", IE_Pressed, this, &AWalkthroughTestCharacter::loadLevelFromJson);
	
	//InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AWalkthroughTestCharacter::TouchStarted);
	if( EnableTouchscreenMovement(InputComponent) == false )
	{
	}
	
	InputComponent->BindAxis("MoveForward", this, &AWalkthroughTestCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AWalkthroughTestCharacter::MoveRight);
	
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AWalkthroughTestCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AWalkthroughTestCharacter::LookUpAtRate);
}

void AWalkthroughTestCharacter::BeginTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if( TouchItem.bIsPressed == true )
	{
		return;
	}
	TouchItem.bIsPressed = true;
	TouchItem.FingerIndex = FingerIndex;
	TouchItem.Location = Location;
	TouchItem.bMoved = false;
}

void AWalkthroughTestCharacter::EndTouch(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if (TouchItem.bIsPressed == false)
	{
		return;
	}
	if( ( FingerIndex == TouchItem.FingerIndex ) && (TouchItem.bMoved == false) )
	{
	}
	TouchItem.bIsPressed = false;
}

void AWalkthroughTestCharacter::TouchUpdate(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	if ((TouchItem.bIsPressed == true) && ( TouchItem.FingerIndex==FingerIndex))
	{
		if (TouchItem.bIsPressed)
		{
			if (GetWorld() != nullptr)
			{
				UGameViewportClient* ViewportClient = GetWorld()->GetGameViewport();
				if (ViewportClient != nullptr)
				{
					FVector MoveDelta = Location - TouchItem.Location;
					FVector2D ScreenSize;
					ViewportClient->GetViewportSize(ScreenSize);
					FVector2D ScaledDelta = FVector2D( MoveDelta.X, MoveDelta.Y) / ScreenSize;									
					if (ScaledDelta.X != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.X * BaseTurnRate;
						AddControllerYawInput(Value);
					}
					if (ScaledDelta.Y != 0.0f)
					{
						TouchItem.bMoved = true;
						float Value = ScaledDelta.Y* BaseTurnRate;
						AddControllerPitchInput(Value);
					}
					TouchItem.Location = Location;
				}
				TouchItem.Location = Location;
			}
		}
	}
}

void AWalkthroughTestCharacter::MoveForward(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorForwardVector(), Value * MoveSpeed);
	}
}

void AWalkthroughTestCharacter::MoveRight(float Value)
{
	if (Value != 0.0f)
	{
		// add movement in that direction
		AddMovementInput(GetActorRightVector(), Value * MoveSpeed);
	}
}

void AWalkthroughTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AWalkthroughTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

bool AWalkthroughTestCharacter::EnableTouchscreenMovement(class UInputComponent* InputComponent)
{
	bool bResult = false;
	if(FPlatformMisc::GetUseVirtualJoysticks() || GetDefault<UInputSettings>()->bUseMouseForTouch )
	{
		bResult = true;
		InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AWalkthroughTestCharacter::BeginTouch);
		InputComponent->BindTouch(EInputEvent::IE_Released, this, &AWalkthroughTestCharacter::EndTouch);
		InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AWalkthroughTestCharacter::TouchUpdate);
	}
	return bResult;
}

void AWalkthroughTestCharacter::saveLevelToJson()
{
	UE_LOG(LogTemp, Warning, TEXT("JSON SAVE!!!"));
	// Only try to save the level if there's some furniture to save, ya dingus
	if (placedFurniture.Num() == 0)
	{
		return;
	}

	// Generate a json string that contains all of the placed furniture
	FString furnitureJsonString;
	furnitureJsonString += "{";
	for (int furnitureIndex = 0; furnitureIndex < placedFurniture.Num(); furnitureIndex++)
	{
		FString name(placedFurniture[furnitureIndex]->GetClass()->GetName());
		name.RemoveFromEnd("_C");
		furnitureJsonString += "\"" + FString::FromInt(furnitureIndex) + "\":\"" + name + "\"";

		if (furnitureIndex == placedFurniture.Num() - 1)
		{
			furnitureJsonString += "}";
		}
		else
		{
			furnitureJsonString += ',';
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Json string to write: %s"), *furnitureJsonString);

	// Write the json string onto a document
	rapidjson::Document d;
	char* json = TCHAR_TO_ANSI(*furnitureJsonString);
	d.Parse(json);

	// Write the json object to a file
	FILE* fp = fopen("D:/AldenHiggins/Projects/WalkthroughTest/Content/FirstPerson/SavedLevels/output.json", "wb"); // non-Windows use "w"
	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
	d.Accept(writer);
	fclose(fp);
}

void AWalkthroughTestCharacter::loadLevelFromJson()
{
	UE_LOG(LogTemp, Warning, TEXT("JSON LOAD!!!"));
	// Read in the json object
	FILE* fp = fopen("D:/AldenHiggins/Projects/WalkthroughTest/Content/FirstPerson/SavedLevels/output.json", "rb"); // non-Windows use "r"
	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
	rapidjson::Document d;
	d.ParseStream(is);
	fclose(fp);

	for (rapidjson::Value::ConstMemberIterator itr = d.MemberBegin(); itr != d.MemberEnd(); ++itr)
	{
		FString theName(d[itr->name.GetString()].GetString());
		UE_LOG(LogTemp, Warning, TEXT("Member name: %s"), *theName);

		FString fullBlueprintName = "Blueprint'/Game/FirstPerson/Blueprints/Furniture/" + theName + "." + theName + "'";
		FStringAssetReference itemRef(fullBlueprintName);
		UObject* itemObj = itemRef.ResolveObject();
		UBlueprint* gen = Cast<UBlueprint>(itemObj);
		FVector NewLocation = FVector(1200.f, 410.f, -10.f);
		AActor* item = GetWorld()->SpawnActor<AActor>(gen->GeneratedClass, NewLocation, FRotator::ZeroRotator);
	}

	//// Print out the json object
	//const char *project = d["project"].GetString();
	//FString projectName(project);

	//UE_LOG(LogTemp, Warning, TEXT("JSON LOAD finished: %s"), *projectName);	

	////FString blueprintName("ShortChair_Blueprint");
	//FString fullBlueprintName = "Blueprint'/Game/FirstPerson/Blueprints/Furniture/" + projectName + "." + projectName + "'";
	////FStringAssetReference itemRef("Blueprint'/Game/FirstPerson/Blueprints/Furniture/BigCouch_Blueprint.BigCouch_Blueprint'");
	//FStringAssetReference itemRef(fullBlueprintName);

	//UObject* itemObj = itemRef.ResolveObject();

	//UBlueprint* gen = Cast<UBlueprint>(itemObj);

	//FVector NewLocation = FVector(1200.f, 410.f, -10.f);

	//AActor* item = GetWorld()->SpawnActor<AActor>(gen->GeneratedClass, NewLocation, FRotator::ZeroRotator);
}

void AWalkthroughTestCharacter::AddObjectToFurnitureList(AActor *newFurniture)
{
	UE_LOG(LogTemp, Warning, TEXT("Added onto this foo!"));
	placedFurniture.Add(newFurniture);
}


