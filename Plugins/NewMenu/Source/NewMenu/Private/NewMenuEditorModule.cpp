#include "NewMenuEditorModule.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Editor/MaterialEditor/Public/IMaterialEditor.h"
#include "Toolkits/ToolkitManager.h"

#undef WITH_EDITORONLY_DATA
#define WITH_EDITORONLY_DATA 1

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

	MainMenuExtender->AddMenuBarExtension( "Help",
		EExtensionHook::After,
		NULL,
		FMenuBarExtensionDelegate::CreateStatic(&NewMenuEditorModule::OnCreateNewMenu2));

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

void NewMenuEditorModule::OnCreateNewMenu2(FMenuBarBuilder& InMenuBarBuilder)
{
	InMenuBarBuilder.AddPullDownMenu
	(
		LOCTEXT("NewMenuEditor", "Material Color Change"),
		LOCTEXT("NewMenuEditorTooltip", "Prepare material to have it's color changed"),
		FNewMenuDelegate::CreateStatic(&NewMenuEditorModule::colorChangeMaterial)
	);
}

void NewMenuEditorModule::colorChangeMaterial(FMenuBuilder& InMenuBarBuilder)
{
	// Get the selected asset from the content browser
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FAssetData> selectedAssets;
	ContentBrowserModule.Get().GetSelectedAssets(selectedAssets);
	// Exit out if the user hasn't selected anything
	if (selectedAssets.Num() == 0)
	{
		return;
	}

	// Get the selected material
	UMaterial* selected = (UMaterial *)selectedAssets[0].GetAsset();
	// Get the material expressions
	TArray<class UMaterialExpression*> *expressions = &selected->Expressions;
	// Exit out if no expressions can be found
	if (expressions->Num() == 0)
	{
		return;
	}

	FColorMaterialInput baseColor = selected->BaseColor;
	UMaterialExpression *colorExpression = baseColor.Expression;

	// Get the color change material function
	FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UMaterialFunction *materialFunction = LoadObject<UMaterialFunction>(NULL, TEXT("/Game/FirstPerson/Materials/Functions/ApplyColorChange.ApplyColorChange"), NULL, LOAD_None, NULL);
	// Create the color change expression
	UMaterialExpressionMaterialFunctionCall* newFunctionCall = NewObject<UMaterialExpressionMaterialFunctionCall>(selected);
	newFunctionCall->MaterialExpressionEditorX = 400;
	newFunctionCall->MaterialExpressionEditorY = 0;
	newFunctionCall->MaterialFunction = materialFunction;
	expressions->Add(newFunctionCall);

	// Create the scalar parameters for the material
	UMaterialExpressionScalarParameter *uCoordinateScalar = NewObject<UMaterialExpressionScalarParameter>((UObject *)selected, UMaterialExpressionScalarParameter::StaticClass(), NAME_None, RF_Transactional);
	uCoordinateScalar->MaterialExpressionEditorX = 200;
	uCoordinateScalar->MaterialExpressionEditorY = 0;
	uCoordinateScalar->ParameterName = "ColorWheelU";
	selected->Expressions.Add(uCoordinateScalar);

	UMaterialExpressionScalarParameter *vCoordinateScalar = NewObject<UMaterialExpressionScalarParameter>((UObject *)selected, UMaterialExpressionScalarParameter::StaticClass(), NAME_None, RF_Transactional);
	vCoordinateScalar->MaterialExpressionEditorX = 200;
	vCoordinateScalar->MaterialExpressionEditorY = 200;
	vCoordinateScalar->ParameterName = "ColorWheelV";
	selected->Expressions.Add(vCoordinateScalar);

	UMaterialExpressionScalarParameter *colorBlendScalar = NewObject<UMaterialExpressionScalarParameter>((UObject *)selected, UMaterialExpressionScalarParameter::StaticClass(), NAME_None, RF_Transactional);
	colorBlendScalar->MaterialExpressionEditorX = 200;
	colorBlendScalar->MaterialExpressionEditorY = 400;
	colorBlendScalar->ParameterName = "ColorBlendAlpha";
	selected->Expressions.Add(colorBlendScalar);


	colorBlendScalar

	for (int expressionIndex = 0; expressionIndex < expressions->Num(); expressionIndex++)
	{
		UMaterialExpression *expression = (*expressions)[expressionIndex];
		FName name = expression->GetFName();
		UE_LOG(LogTemp, Warning, TEXT("EXP Name: %s"), *(name.ToString()));
		
		if (expression == colorExpression)
		{
			UE_LOG(LogTemp, Warning, TEXT("The color input expression yo yo yo"));
		}

		// Print the inputs of the expression
		TArray<FExpressionInput*> inputs = expression->GetInputs();
		for (int inputIndex = 0; inputIndex < inputs.Num(); inputIndex++)
		{
			UE_LOG(LogTemp, Warning, TEXT("Input name: %s"), *inputs[inputIndex]->InputName);
		}

		// Print the outputs of the expression
		TArray<FExpressionOutput> outputs = expression->GetOutputs();
		for (int outputIndex = 0; outputIndex < outputs.Num(); outputIndex++)
		{
			UE_LOG(LogTemp, Warning, TEXT("Output name: %s"), *outputs[outputIndex].OutputName);
		}
	}

	
	UE_LOG(LogTemp, Warning, TEXT("Expressisons found!!!"));
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



