// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include "ShareTexturesData.generated.h"

/**
 * Data structure to hold preview image data based on the backend model
 */
USTRUCT()
struct FShareTexturesPreviewImage
{
	GENERATED_BODY()

	UPROPERTY()
	FString ThumbObjectKey;
};

/**
 * Data structure to hold tag data based on the backend model
 */
USTRUCT()
struct FShareTexturesTag
{
	GENERATED_BODY()

	UPROPERTY()
	FString Slug;

	UPROPERTY()
	FString Text;
};

/**
 * Data structure to hold item type data based on the backend model
 */
USTRUCT()
struct FShareTexturesItemType 
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;
};

/**
 * Data structure to hold download link data based on the backend model
 */
USTRUCT()
struct FShareTexturesDownloadLink
{
	GENERATED_BODY()

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString Value;
};

/**
 * Data structure to hold category data based on the backend model
 */
USTRUCT()
struct FShareTexturesCategoryData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;
};

/**
 * Data structure to hold item data based on the backend model
 */
USTRUCT()
struct FShareTexturesItemData
{
	GENERATED_BODY()

	UPROPERTY()
	FString _id;

	UPROPERTY()
	FString Title;

	UPROPERTY()
	FString Slug;

	UPROPERTY()
	FShareTexturesItemType ItemType;

	UPROPERTY()
	FShareTexturesCategoryData Category;

	UPROPERTY()
	FShareTexturesPreviewImage PreviewImage1;

	UPROPERTY()
	FDateTime PublishDate;

	UPROPERTY()
	TArray<FShareTexturesDownloadLink> DownloadLinks;

	UPROPERTY()
	TArray<FShareTexturesTag> Tags;
};

/**
 * Wrapper class to hold a list item data
 */
UCLASS(BlueprintType, MinimalAPI)
class UShareTexturesItem : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<FShareTexturesItemData> ItemData;

	FString CurrentResolution;

	FString GetDownloadLink() const;
	FString GetCacheLocation() const;
	FString GetImportPath() const;
	FString GetResolution() const;
	FString GetCategory() const;
	FString GetSlug() const;
	FString GetTitle() const;
	bool IsCached() const;
};

/**
 * Wrapper class to hold a list item category data
 */
UCLASS(BlueprintType, MinimalAPI)
class UShareTexturesCategory : public UObject
{
	GENERATED_BODY()

public:
	TSharedPtr<FShareTexturesCategoryData> CategoryData;

	int ItemCount;
};
