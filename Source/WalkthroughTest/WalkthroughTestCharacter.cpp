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
#include "rapidjson/error/en.h"
#include "Runtime/CoreUObject/Public/UObject/UnrealType.h"
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
	furnitureJsonString += "{ ";
	for (int furnitureIndex = 0; furnitureIndex < placedFurniture.Num(); furnitureIndex++)
	{
		FString name(placedFurniture[furnitureIndex]->GetClass()->GetName());
		name.RemoveFromEnd("_C");
		// Save the piece of furniture under the index it appears in the furniture array
		furnitureJsonString += "\"" + FString::FromInt(furnitureIndex) + "\":";

		// Save the name of the furniture
		furnitureJsonString += "{";
		furnitureJsonString += "\"";
		furnitureJsonString += "Name";
		furnitureJsonString += "\"";
		furnitureJsonString += ":";

		furnitureJsonString += "\"";
		furnitureJsonString += name;
		furnitureJsonString += "\"";
		furnitureJsonString += ", ";

		// Save the position of the furniture
		furnitureJsonString += "\"";
		furnitureJsonString += "Position";
		furnitureJsonString += "\"";
		furnitureJsonString += ":";
		furnitureJsonString += "[";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorLocation().X);
		furnitureJsonString += ", ";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorLocation().Y);
		furnitureJsonString += ", ";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorLocation().Z);
		furnitureJsonString += "]";
		furnitureJsonString += ", ";

		// Save the rotation of the furniture
		furnitureJsonString += "\"";
		furnitureJsonString += "Rotation";
		furnitureJsonString += "\"";
		furnitureJsonString += ":";
		furnitureJsonString += "[";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorRotation().Euler().X);
		furnitureJsonString += ", ";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorRotation().Euler().Y);
		furnitureJsonString += ", ";
		furnitureJsonString += FString::SanitizeFloat(placedFurniture[furnitureIndex]->GetActorRotation().Euler().Z);
		furnitureJsonString += "]";

		furnitureJsonString += "}";

		if (furnitureIndex == placedFurniture.Num() - 1)
		{
			furnitureJsonString += " }";
		}
		else
		{
			furnitureJsonString += ", ";
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Json string to write: %s"), *furnitureJsonString);

	// Write the json string onto a document
	rapidjson::Document d;
	std::string string = TCHAR_TO_UTF8(*furnitureJsonString);

	if (d.Parse(string.c_str()).HasParseError()) 
	{
		FString name(rapidjson::GetParseError_En(d.GetParseError()));
		UE_LOG(LogTemp, Warning, TEXT("Error location index: %d"), d.GetErrorOffset());
		UE_LOG(LogTemp, Warning, TEXT("Parsing error: %s"), *name);
	}

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
		// Spawn the piece of furniture
		FString furnitureBlueprintName(d[itr->name.GetString()]["Name"].GetString());
		FString fullBlueprintName = "Blueprint'/Game/FirstPerson/Blueprints/Furniture/" + furnitureBlueprintName + "." + furnitureBlueprintName + "'";
		FStringAssetReference itemRef(fullBlueprintName);
		UObject* itemObj = itemRef.ResolveObject();
		UBlueprint* gen = Cast<UBlueprint>(itemObj);
		FVector NewLocation = FVector(1200.f, 410.f, -10.f);
		AActor* item = GetWorld()->SpawnActor<AActor>(gen->GeneratedClass, NewLocation, FRotator::ZeroRotator);

		

		//UFloatProperty* FloatProp = FindField<UFloatProperty>(item->GetClass(), *fieldName);
		//if (FloatProp != NULL)
		//{
		//	//GetFloatingPointPropertyValue
		//	//float* FloatPtr = FloatProp->GetPropertyValue_InContainer(item);
		//	double FloatPtr = FloatProp->GetFloatingPointPropertyValue(item);

		//	UE_LOG(LogTemp, Warning, TEXT("Alpha val: %f"), FloatPtr);

		//}

		TArray<UActorComponent *> components = item->GetComponents();
		for (int componentIndex = 0; componentIndex < components.Num(); componentIndex++)
		{
			FString componentName = components[componentIndex]->GetName();
			UE_LOG(LogTemp, Warning, TEXT("Component name: %s"), *components[componentIndex]->GetName());
			if (componentName == "InteractiveObject")
			{
				UE_LOG(LogTemp, Warning, TEXT("Interactive found!!!"));
			}
		}

		FString fieldName("MaterialColorAlpha");
		UFloatProperty* MyFloatProp = FindField<UFloatProperty>((UClass *)gen->GeneratedClass, *fieldName);
		if (MyFloatProp != NULL)
		{
			float FloatVal = MyFloatProp->GetPropertyValue_InContainer(item);
			UE_LOG(LogTemp, Warning, TEXT("Alpha val: %f"), FloatVal);

			//MyFloatProp->SetPropertyValue_InContainer(item, 180.0f);
			//FloatVal = MyFloatProp->GetPropertyValue_InContainer(item);
		}
		

		// Place it in the correct location
		const rapidjson::Value& furnitureInfoArray = d[itr->name.GetString()]["Position"];
		assert(furnitureInfoArray.IsArray());
		if (furnitureInfoArray.Size() >= 3)
		{
			FVector position(furnitureInfoArray[0].GetDouble(), furnitureInfoArray[1].GetDouble(), furnitureInfoArray[2].GetDouble());
			item->SetActorLocation(position);
		}

		// Rotate the furniture
		const rapidjson::Value& rotationInfo = d[itr->name.GetString()]["Rotation"];
		assert(furnitureInfoArray.IsArray());
		if (furnitureInfoArray.Size() >= 3)
		{
			FRotator rotation = FRotator::MakeFromEuler(FVector(rotationInfo[0].GetDouble(), rotationInfo[1].GetDouble(), rotationInfo[2].GetDouble()));
			item->SetActorRotation(rotation);
		}

		// Add the spawned furniture to placedfurniture
		placedFurniture.Add(item);

		//for (rapidjson::SizeType i = 0; i < furnitureInfoArray.Size(); i++)
		//{
		//	double printThis = furnitureInfoArray[i].GetDouble();
		//	UE_LOG(LogTemp, Warning, TEXT("Member value: %f"), printThis);
		//}
	}
}

void AWalkthroughTestCharacter::AddObjectToFurnitureList(AActor *newFurniture)
{
	placedFurniture.Add(newFurniture);
}

void AWalkthroughTestCharacter::RemoveObjectFromFurnitureList(AActor *removeThis)
{
	placedFurniture.Remove(removeThis);
}


