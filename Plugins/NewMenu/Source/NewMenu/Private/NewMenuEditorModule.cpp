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
		LOCTEXT( "NewMenuEditor", "Create Thumb Mat" ),
		LOCTEXT( "NewMenuEditorTooltip", "Generate a new thumbnail material from the selected texture." ),
		FNewMenuDelegate::CreateStatic(&NewMenuEditorModule::createThumbnailMaterialInstance)
	);
}

void NewMenuEditorModule::createThumbnailMaterialInstance(FMenuBuilder& InMenuBarBuilder)
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> selectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(selectedAssets);

	if (selectedAssets.Num() == 0)
	{
		return;
	}
	
	// Get the thumbnail material that we will create an instance of
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UMaterial *TheMaterial = LoadObject<UMaterial>(NULL, TEXT("/Game/FirstPerson/Materials/FurnitureIcons/ThumbnailMaterial.ThumbnailMaterial"), NULL, LOAD_None, NULL);

	// Get the package name of the thumbnail material
	FString Name;
	FString PackageName;
	AssetToolsModule.Get().CreateUniqueAssetName(TheMaterial->GetOutermost()->GetName(), TEXT("_Mat"), PackageName, Name);
	
	// Add _Mat to the name of the thumbnail image
	UTexture* selected = (UTexture *) selectedAssets[0].GetAsset();
	FString selectedName = selectedAssets[0].AssetName.ToString();
	selectedName += "_Mat";

	// Create the new material instance
	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	Factory->InitialParent = CastChecked<UMaterialInterface>(TheMaterial);
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(selectedName, FPackageName::GetLongPackagePath(PackageName), UMaterialInstanceConstant::StaticClass(), Factory);
	// Set the texture parameter to be the selected texture
	UMaterialInstanceConstant *newlyCreatedMaterial = (UMaterialInstanceConstant *)NewAsset;
	newlyCreatedMaterial->SetTextureParameterValueEditorOnly("ThumbTexture", selected);
}



