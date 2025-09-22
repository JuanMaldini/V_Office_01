// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include "ShareTexturesSubsystem.generated.h"

struct FShareTexturesCategoryData;
struct FShareTexturesItemData;

class IHttpRequest;
class IHttpResponse;

template <typename T>
using TResponseDataDelegate = TDelegate<void(TArray<T>)>;

/**
 * Subsystem to centralize the caching systems and server communication 
 */
UCLASS()
class UShareTexturesSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	/**
	 * Tries to get a texture from the cache for a specific image.
	 * 1) First we try to get it from the InMemoryTextures caches
	 * 2) If it's not there, we try to load it from the disk
	 * If it's not there, we return nullptr
	 */
	UTexture2DDynamic* GetCachedImageData(const FString& InImage);
	/**
	 * Saves an image's data to the cache on disk, so future requests can access it quicker
	 */
	void CacheImageDataOnDisk(const FString& InImage, const TArray<uint8>& InData);
	/**
	 * Removes all the in-memory textures we have stored, to ensure future queries are fresh 
	 */
	void ClearInMemoryCache();
	/**
	 * Sends a request to the backend to fetch all the items available
	 */
	void SendFetchItemsRequest(TResponseDataDelegate<FShareTexturesItemData> OnItemsFetched);
	/**
	 * Sends a request to the backend to fetch all the categories available
	 */
	void SendFetchCategoriesRequest(TResponseDataDelegate<FShareTexturesCategoryData> OnCategoriesFetched);
	/**
	 * Gets the cached location (if any) for a specific product info combination
	 */
	FString GetTextureDataCacheLocation(const FString& InSlug, const FString& InResolution, const FString& InCategory) const;

private:
	/**
	 * Helper function to load the binary bytes data into a texture
	 */
	UTexture2DDynamic* BytesToTexture(const TArray<uint8>& InData);
	/**
	 * Callback executed when the items are fetched
	 */
	void OnItemsFetchedResponse(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess, TResponseDataDelegate<FShareTexturesItemData> OnItemsFetched);
	/**
	 * Callback executed when the categories are fetched
	 */
	void OnCategoriesFetchedResponse(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess, TResponseDataDelegate<FShareTexturesCategoryData> OnCategoriesFetched);
	/**
	 * InMemory cache of textures we have already loaded mapped to image's slug
	 */
	UPROPERTY()
	TMap<FString, UTexture2DDynamic*> InMemoryTextures;
};