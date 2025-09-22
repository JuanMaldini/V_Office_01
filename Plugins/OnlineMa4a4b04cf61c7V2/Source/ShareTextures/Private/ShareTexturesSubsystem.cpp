// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesSubsystem.h"

#include <HttpModule.h>
#include <IImageWrapper.h>
#include <IImageWrapperModule.h>
#include <JsonObjectConverter.h>
#include <Engine/Texture2DDynamic.h>
#include <Interfaces/IHttpResponse.h>

#include "ShareTexturesData.h"
#include "ShareTexturesSettings.h"

namespace
{
	template <typename OutStructType>
	bool JsonStringToUStructArray(FString const& InString, TArray<OutStructType>& OutStructArray)
	{
		TSharedRef<TJsonReader<>> const JsonReader = TJsonReaderFactory<>::Create(InString);
		TArray<TSharedPtr<FJsonValue>> JsonValueArray;
		if (!FJsonSerializer::Deserialize(JsonReader, JsonValueArray))
		{
			return false;
		}

		for (TSharedPtr<FJsonValue> const& Value : JsonValueArray)
		{
			OutStructType OutStruct;
			if (!FJsonObjectConverter::JsonObjectToUStruct<OutStructType>(Value->AsObject().ToSharedRef(), &OutStruct))
			{
				return false;
			}

			OutStructArray.Add(OutStruct);
		}
		return true;
	}
}

UTexture2DDynamic* UShareTexturesSubsystem::GetCachedImageData(const FString& InImage)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::GetCachedImageData);

	if(UTexture2DDynamic** Texture = InMemoryTextures.Find(InImage))
	{
		return *Texture;
	}

	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString ThumbnailLocation = Settings->CacheLocation / TEXT("Thumbnails") / InImage;

	if(!IFileManager::Get().FileExists(*ThumbnailLocation))
	{
		return nullptr;
	}

	TArray<uint8> Content;
	FFileHelper::LoadFileToArray(Content, *ThumbnailLocation);

	UTexture2DDynamic* Texture = BytesToTexture(Content);
	if (Texture)
	{
		InMemoryTextures.Add(InImage, Texture);
	}

	return Texture;
}

void UShareTexturesSubsystem::CacheImageDataOnDisk(const FString& InImage, const TArray<uint8>& InData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::SetCachedImageData);

	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString ThumbnailLocation = Settings->CacheLocation / TEXT("Thumbnails") / InImage;

	FFileHelper::SaveArrayToFile(InData, *ThumbnailLocation);
}

void UShareTexturesSubsystem::ClearInMemoryCache()
{
	InMemoryTextures.Empty();
}

void UShareTexturesSubsystem::SendFetchItemsRequest(TResponseDataDelegate<FShareTexturesItemData> OnItemsFetched)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::SendFetchItemsRequest);

	const FString Endpoint = TEXT("https://api2.sharetextures.com/api/v0/item/s?limit=9999");

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Endpoint);
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &UShareTexturesSubsystem::OnItemsFetchedResponse, MoveTemp(OnItemsFetched));

	Request->ProcessRequest();
}

void UShareTexturesSubsystem::SendFetchCategoriesRequest(TResponseDataDelegate<FShareTexturesCategoryData> OnCategoriesFetched)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::SendFetchCategoriesRequest);

	const FString Url = TEXT("https://api2.sharetextures.com/api/v0/category/list/by/63334c7b180bb34fa67996e4");

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(Url);
	Request->SetVerb(TEXT("GET"));
	Request->OnProcessRequestComplete().BindUObject(this, &UShareTexturesSubsystem::OnCategoriesFetchedResponse, MoveTemp(OnCategoriesFetched));

	Request->ProcessRequest();
}

FString UShareTexturesSubsystem::GetTextureDataCacheLocation(const FString& InSlug, const FString& InResolution, const FString& InCategory) const
{
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const FString ImportFolderPath = Settings->CacheLocation / InCategory / InResolution / InSlug;

	if (!IFileManager::Get().DirectoryExists(*ImportFolderPath))
	{
		return {};
	}

	return ImportFolderPath;
}

UTexture2DDynamic* UShareTexturesSubsystem::BytesToTexture(const TArray<uint8>& InData)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::BytesToTexture);

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	TSharedPtr<IImageWrapper> ImageWrappers[3] =
	{
		ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG),
		ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG),
		ImageWrapperModule.CreateImageWrapper(EImageFormat::BMP),
	};

	for (TSharedPtr<IImageWrapper> ImageWrapper : ImageWrappers)
	{
		if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(InData.GetData(), InData.Num()))
		{
			TArray64<uint8> RawData;
			constexpr ERGBFormat InFormat = ERGBFormat::BGRA;
			if (ImageWrapper->GetRaw(InFormat, 8, RawData))
			{
				if (UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()))
				{
					Texture->SRGB = true;
					Texture->UpdateResource();

					if (FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource()))
					{
						ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
							[TextureResource, RawData = MoveTemp(RawData)](FRHICommandListImmediate& RHICmdList)
							{
								TextureResource->WriteRawToTexture_RenderThread(RawData);
							});
					}

					return Texture;
				}
			}
		}
	}

	return nullptr;
}

void UShareTexturesSubsystem::OnItemsFetchedResponse(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess, TResponseDataDelegate<FShareTexturesItemData> OnItemsFetched)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::OnItemsFetchedResponse);

	const FString ResponseData = Response->GetContentAsString();

	TArray<FShareTexturesItemData> Items;
	JsonStringToUStructArray(ResponseData, Items);

	(void) OnItemsFetched.ExecuteIfBound(Items);
}

void UShareTexturesSubsystem::OnCategoriesFetchedResponse(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess, TResponseDataDelegate<FShareTexturesCategoryData> OnCategoriesFetched)
{
	TRACE_CPUPROFILER_EVENT_SCOPE(UShareTexturesSubsystem::OnCategoriesFetchedResponse);

	const FString ResponseData = Response->GetContentAsString();

	TArray<FShareTexturesCategoryData> Categories;
	JsonStringToUStructArray(ResponseData, Categories);

	(void) OnCategoriesFetched.ExecuteIfBound(Categories);
}