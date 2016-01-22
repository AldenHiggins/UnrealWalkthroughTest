#include "NewMenuEditorModule.h"

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
	UE_LOG(LogTemp, Warning, TEXT("A different message"));
}
