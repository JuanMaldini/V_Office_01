// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesData.h"

#include "ShareTexturesSettings.h"

FString UShareTexturesItem::GetDownloadLink() const
{
	const FString Resolution = GetResolution();
	const FShareTexturesDownloadLink* DownloadLink = ItemData->DownloadLinks.FindByPredicate([Resolution](const FShareTexturesDownloadLink& Item)
	{
		return Item.Title == Resolution;
	});
	if (DownloadLink)
	{
		return DownloadLink->Value;
	}

	return {};
}

FString UShareTexturesItem::GetCacheLocation() const
{
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString ImportFolderPath = Settings->CacheLocation / GetCategory() / GetResolution() / GetSlug();

	return ImportFolderPath;
}

FString UShareTexturesItem::GetImportPath() const
{
	const FString ImportFolder = TEXT("/Game/ShareTextures") / GetCategory() / GetResolution();
	return ImportFolder;
}

FString UShareTexturesItem::GetResolution() const
{
	return CurrentResolution;
}

FString UShareTexturesItem::GetCategory() const
{
	return ItemData->Category.Name;
}

FString UShareTexturesItem::GetSlug() const
{
	return ItemData->Slug;
}

FString UShareTexturesItem::GetTitle() const
{
	return ItemData->Title;
}

bool UShareTexturesItem::IsCached() const
{
	return IFileManager::Get().DirectoryExists(*GetCacheLocation());
}