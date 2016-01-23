#include "NewMenuEditorModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
//#include "Engine/EngineTypes.h"

IMPLEMENT_MODULE( NewMenuEditorModule, NewMenu );

void NewMenuEditorModule::StartupModule()
{
	CreateNewMenu();
}

void NewMenuEditorModule::ShutdownModule()
{

}

void NewMenuEditorModule::CreateNewMenu()
{
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>( "LevelEditor" );
	MainMenuExtender = MakeShareable( new FExtender );
	MainMenuExtender->AddMenuBarExtension( "Help",
										EExtensionHook::After,
										NULL,
										FMenuBarExtensionDelegate::CreateStatic( &NewMenuEditorModule::OnCreateNewMenu ) );

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender( MainMenuExtender );
}

void NewMenuEditorModule::OnCreateNewMenu( FMenuBarBuilder& InMenuBarBuilder )
{
	InMenuBarBuilder.AddPullDownMenu
	(
		LOCTEXT( "NewMenuEditor", "Alden Action" ),
		LOCTEXT( "NewMenuEditorTooltip", "Should do something cool?" ),
		FNewMenuDelegate::CreateStatic(&NewMenuEditorModule::testMethod)
	);
}

void NewMenuEditorModule::testMethod(FMenuBuilder& InMenuBarBuilder)
{
	UE_LOG(LogTemp, Warning, TEXT("Started alden method"));
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UMaterial *TheMaterial = LoadObject<UMaterial>(NULL, TEXT("/Game/FirstPerson/Materials/FurnitureIcons/ThumbnailMaterial.ThumbnailMaterial"), NULL, LOAD_None, NULL);

	// Create an appropriate and unique name
	FString Name;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(TheMaterial->GetOutermost()->GetName(), TEXT("_Mat"), PackageName, Name);
	UE_LOG(LogTemp, Warning, TEXT("New material name: %s and packageName: %s"), *Name, *PackageName);

	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = CastChecked<UMaterialInterface>(TheMaterial);

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> selectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(selectedAssets);

	UE_LOG(LogTemp, Warning, TEXT("Selected assets size: %d"), selectedAssets.Num());

	if (selectedAssets.Num() > 0)
	{
		UTexture* selected = (UTexture *) selectedAssets[0].GetAsset();

		//UObject* selected = selectedAssets[0].GetAsset();

		if (selected->IsNormalMap() == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("Not normal map"));
		}
		FString selectedName = selectedAssets[0].AssetName.ToString();
		UE_LOG(LogTemp, Warning, TEXT("Name of selected asset: %s"), *selectedName);

		selectedName += "_Mat";
		UE_LOG(LogTemp, Warning, TEXT("Name of selected asset: %s"), *selectedName);

		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(selectedName, FPackageName::GetLongPackagePath(PackageName), UMaterialInstanceConstant::StaticClass(), Factory);

		UMaterialInstanceConstant *newlyCreatedMaterial = (UMaterialInstanceConstant *)NewAsset;
		UE_LOG(LogTemp, Warning, TEXT("Got here"));

		newlyCreatedMaterial->SetTextureParameterValueEditorOnly("ThumbTexture", selected);
	}

	UE_LOG(LogTemp, Warning, TEXT("Finished alden method"));
}



