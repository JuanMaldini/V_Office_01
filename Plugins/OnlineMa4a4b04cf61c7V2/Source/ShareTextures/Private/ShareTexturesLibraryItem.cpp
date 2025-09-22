// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesLibraryItem.h"

#include <HttpModule.h>
#include <Components/Image.h>
#include <Components/TextBlock.h>
#include <Engine/Texture2DDynamic.h>
#include <Interfaces/IHttpResponse.h>

#include "ShareTexturesData.h"
#include "ShareTexturesSubsystem.h"

void UShareTexturesLibraryItem::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	UShareTexturesItem* TextureItem = GetListItem<UShareTexturesItem>();
	if (!TextureItem)
	{
		return;
	}

	const TSharedPtr<FShareTexturesItemData>& TextureItemData = TextureItem->ItemData;
	if (ensure(Title))
	{
		Title->SetText(FText::FromString(TextureItemData->Title));
	}

	bDownloadStarted = false;
	Background->SetBrushResourceObject(nullptr);
}

bool UShareTexturesLibraryItem::IsCachedOnDisk() const
{
	UShareTexturesItem* TextureItem = GetListItem<UShareTexturesItem>();
	if (!TextureItem)
	{
		return false;
	}

	return TextureItem->IsCached();
}

bool UShareTexturesLibraryItem::IsRecentlyPublished() const
{
	UShareTexturesItem* TextureItem = GetListItem<UShareTexturesItem>();
	if (!TextureItem)
	{
		return false;
	}

	const FTimespan TimeSincePublish = FDateTime::Now() - TextureItem->ItemData->PublishDate;
	return TimeSincePublish < FTimespan::FromDays(3);
}

void UShareTexturesLibraryItem::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().FindWidgetWindow(GetCachedWidget().ToSharedRef());
	if (!ParentWindow || !ParentWindow->GetCachedGeometry().IsUnderLocation(MyGeometry.GetAbsolutePosition()))
	{
		return;
	}

	if (!bDownloadStarted)
	{
		DownloadImage();
		bDownloadStarted = true;
	}
}

void UShareTexturesLibraryItem::DownloadImage()
{
	UShareTexturesItem* TextureItem = GetListItem<UShareTexturesItem>();
	if (!TextureItem)
	{
		return;
	}

	const TSharedPtr<FShareTexturesItemData>& TextureItemData = TextureItem->ItemData;
	UShareTexturesSubsystem* Subsystem = GEditor->GetEditorSubsystem<UShareTexturesSubsystem>();
	if (UTexture2DDynamic* CachedImage = Subsystem->GetCachedImageData(TextureItemData->Slug))
	{
		Background->SetBrushResourceObject(CachedImage);
	}
	else
	{
		FString ImageUrl = TEXT("https://images.sharetextures.com") / TextureItemData->PreviewImage1.ThumbObjectKey;

		// WEBP format cannot be imported as a texture in Unreal, therefore we added PNGs to simplify the process
		// However, we couldn't add the PNG url in the request data, so we are just adjusting the URL to point to the new one
		ImageUrl = ImageUrl.Replace(TEXT(".webp"), TEXT(".png"));
		ImageUrl = ImageUrl.Replace(TEXT("/u/"), TEXT("/u/original/"));

		FHttpRequestRef HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->OnProcessRequestComplete().BindUObject(this, &UShareTexturesLibraryItem::OnImageDownloaded);
		HttpRequest->SetURL(ImageUrl);
		HttpRequest->SetVerb(TEXT("GET"));
		HttpRequest->ProcessRequest();
	}
}

void UShareTexturesLibraryItem::OnImageDownloaded(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess)
{
	if (!bSuccess || !Response)
	{
		return;
	}

	UShareTexturesItem* TextureItem = GetListItem<UShareTexturesItem>();
	if (!TextureItem)
	{
		return;
	}

	if (Background->GetBrush().GetResourceObject())
	{
		// We already have a valid texture set, so we don't care about this response anymore 
		return;
	}

	const TSharedPtr<FShareTexturesItemData>& TextureItemData = TextureItem->ItemData;
	const TArray<uint8>& ResponseContent = Response->GetContent();

	UShareTexturesSubsystem* Subsystem = GEditor->GetEditorSubsystem<UShareTexturesSubsystem>();
	Subsystem->CacheImageDataOnDisk(TextureItemData->Slug, ResponseContent);

	UTexture2DDynamic* Texture = Subsystem->GetCachedImageData(TextureItemData->Slug);
	Background->SetBrushResourceObject(Texture);
}