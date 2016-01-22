#include "NewMenuEditorModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"

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
	ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackageName), UMaterialInstanceConstant::StaticClass(), Factory);

	UE_LOG(LogTemp, Warning, TEXT("Finished alden method"));
	//UMaterial *TheMaterial;
	//if (MatFinder.Succeeded())
	//{
	//	if (MatFinder.Object != NULL)
	//	{
	//		TheMaterial = (UMaterial*)MatFinder.Object;
	//		UE_LOG(LogTemp, Warning, TEXT("Material found"));
	//	}
	//}
	



	//if (MatFinder.Succeeded())
	//{
	//	Material = MatFinder.Object;
	//	MaterialInstance = UMaterialInstanceDynamic::Create(Material, this);
	//	//Setting a font parameter of a dynamic material instance crashes the game when invoked in a constructor
	//	//using default font parameter until onconstruction
	//	//DamageTextMaterialInstance->SetFontParameterValue("Font", DamageTextFont, 0);
	//	MaterialInstance->SetScalarParameterValue("Glow amount", CurrentGlowAmount);
	//	MaterialInstance->SetScalarParameterValue("Opacity", CurrentOpacity);
	//}



	// Input:	
	//TArray<TWeakObjectPtr<UMaterialInterface>> Objects

	// Create a material instance from a material
	////auto Object = Objects[0].Get();

	////if (Object)
	////{
	////	// Create an appropriate and unique name 
	////	FString Name;
	////	FString PackageName;
	////	CreateUniqueAssetName(Object->GetOutermost()->GetName(), DefaultSuffix, PackageName, Name);

	////	UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
	////	Factory->InitialParent = Object;

	////	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	////	ContentBrowserModule.Get().CreateNewAsset(Name, FPackageName::GetLongPackagePath(PackageName), UMaterialInstanceConstant::StaticClass(), Factory);
	////}
}



