// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesSettingsDetails.h"

#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>

#include "ShareTexturesLog.h"
#include "ShareTexturesSettings.h"
#include "ShareTexturesSubsystem.h"

#define LOCTEXT_NAMESPACE "ShareTextures"

TSharedRef<IDetailCustomization> FShareTexturesSettingsDetails::MakeInstance()
{
	return MakeShared<FShareTexturesSettingsDetails>();
}

void FShareTexturesSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("ShareTextures");

	Category.AddCustomRow(LOCTEXT("ClearCacheRow", "Clear Cache"))
	        .ValueContent()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("ClearCacheButton", "Clear Cache"))
			.ToolTipText(LOCTEXT("ClearCacheButton_Tooltip", "Deletes the content of the Cache folder to force fresh downloads."))
			.OnClicked(this, &FShareTexturesSettingsDetails::OnClearCacheClicked)
			.IsEnabled(this, &FShareTexturesSettingsDetails::CanClearCache)
		]
	];
}

FReply FShareTexturesSettingsDetails::OnClearCacheClicked()
{
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString CacheLocation = Settings->CacheLocation;

	UE_LOG(LogShareTextures, Display, TEXT("Clearing cache directory at: %s"), *CacheLocation)
	IFileManager::Get().DeleteDirectory(*CacheLocation, false, true);

	UShareTexturesSubsystem* Subsystem = GEditor->GetEditorSubsystem<UShareTexturesSubsystem>();
	Subsystem->ClearInMemoryCache();

	return FReply::Handled();
}

bool FShareTexturesSettingsDetails::CanClearCache() const
{
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString CacheLocation = Settings->CacheLocation;

	return IFileManager::Get().DirectoryExists(*CacheLocation);
}

#undef LOCTEXT_NAMESPACE