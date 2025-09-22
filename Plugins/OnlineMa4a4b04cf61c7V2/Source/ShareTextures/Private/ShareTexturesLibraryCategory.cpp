// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesLibraryCategory.h"

#include <Components/TextBlock.h>

#include "ShareTexturesData.h"

void UShareTexturesLibraryCategory::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UShareTexturesCategory* TextureCategory = GetListItem<UShareTexturesCategory>();
	if (!TextureCategory)
	{
		return;
	}

	const TSharedPtr<FShareTexturesCategoryData>& TextureCategoryData = TextureCategory->CategoryData;
	if (ensure(Title))
	{
		Title->SetText(FText::FromString(TextureCategoryData->Name));
	}

	if (ensure(Count))
	{
		Count->TextDelegate.BindDynamic(this, &UShareTexturesLibraryCategory::GetItemCount);
		Count->SynchronizeProperties();
	}
}

FText UShareTexturesLibraryCategory::GetItemCount()
{
	UShareTexturesCategory* TextureCategory = GetListItem<UShareTexturesCategory>();
	if (!TextureCategory)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(LexToString(TextureCategory->ItemCount));
}