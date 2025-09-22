// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTextures.h"

#include <EditorUtilitySubsystem.h>
#include <EditorUtilityWidgetBlueprint.h>

#include "ShareTexturesSettings.h"
#include "ShareTexturesSettingsDetails.h"
#include "ShareTexturesStyle.h"

#define LOCTEXT_NAMESPACE "ShareTextures"

void FShareTexturesModule::StartupModule()
{
	FShareTexturesStyle::Initialize();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FShareTexturesModule::RegisterMenus));

	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(UShareTexturesSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&FShareTexturesSettingsDetails::MakeInstance));
}

void FShareTexturesModule::ShutdownModule()
{
	FShareTexturesStyle::Shutdown();

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	if (FPropertyEditorModule* PropertyModule = FModuleManager::GetModulePtr<FPropertyEditorModule>("PropertyEditor"))
	{
		PropertyModule->UnregisterCustomClassLayout(UShareTexturesSettings::StaticClass()->GetFName());
	}
}

void FShareTexturesModule::OpenShareTexturesTab()
{
	const UShareTexturesSettings* const Settings = GetDefault<UShareTexturesSettings>();

	if (UEditorUtilityWidgetBlueprint* TabWidget = Settings->TabWidgetBlueprint.Get())
	{
		UEditorUtilitySubsystem* EditorUtilitySubsystem = GEditor->GetEditorSubsystem<UEditorUtilitySubsystem>();
		EditorUtilitySubsystem->SpawnAndRegisterTab(TabWidget);
	}
}

void FShareTexturesModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
	FToolMenuSection& Section = ToolbarMenu->AddSection("ShareTexturesTab");

	FToolMenuEntry ShareTexturesTabEntry = FToolMenuEntry::InitToolBarButton("ShareTexturesTab",
	                                                                         FToolUIActionChoice(FExecuteAction::CreateLambda([this]()
	                                                                         {
		                                                                         OpenShareTexturesTab();
	                                                                         })),
	                                                                         LOCTEXT("ShareTexturesTabLabel", "Share Textures tab"),
	                                                                         LOCTEXT("ShareTexturesTabDescription", "Open the ShareTextures tab."),
	                                                                         FSlateIcon(FShareTexturesStyle::GetStyleSetName(), "ShareTextures.ToolbarIcon")
	);

	ShareTexturesTabEntry.StyleNameOverride = "AssetEditorToolbar";
	Section.AddEntry(ShareTexturesTabEntry);
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShareTexturesModule, ShareTextures)