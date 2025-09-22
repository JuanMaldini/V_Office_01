// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include <Blueprint/IUserObjectListEntry.h>
#include <Blueprint/UserWidget.h>

#include "ShareTexturesLibraryItem.generated.h"

class UImage;
class UTextBlock;
class IHttpResponse;
class IHttpRequest;

/**
 * Widget used to display a single item (texture) in the library 
 */
UCLASS(Abstract)
class UShareTexturesLibraryItem : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	/**
	 * Background image used to display the presentation image
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UImage> Background;
	/**
	 * Title text block used to display the item's name
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;
	/**
	 * Checks if the currently assigned item has been downloaded already and the raw data is available on disk
	 */
	UFUNCTION(BlueprintCallable, Category = "ShareTextures")
	bool IsCachedOnDisk() const;
	/**
	 * Checks if the currently assigned item has been published in the last few days 
	 */
	UFUNCTION(BlueprintCallable, Category = "ShareTextures")
	bool IsRecentlyPublished() const;

private:
	//~ Begin UUserWidget Interface
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	//~ End UUserWidget Interface

	//~ Begin IUserObjectListEntry Interface
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	//~ End IUserObjectListEntry Interface

	/**
	 * Starts the download process for the item's background image or uses the cached image to display it
	 */
	void DownloadImage();
	/**
	 * Callback executed when the item's background is retrieved from the server
	 */
	void OnImageDownloaded(TSharedPtr<IHttpRequest> Request, TSharedPtr<IHttpResponse> Response, bool bSuccess);
	/**
	 * Guard to ensure we only download the item's background image once
	 */
	bool bDownloadStarted = false;
};