// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include <Blueprint/UserWidget.h>

#include "ShareTexturesLibrary.generated.h"

class IHttpResponse;
class IHttpRequest;

class UTileView;
class UListView;
class UButton;
class UEditableText;
class UComboBoxString;

class UShareTexturesItem;
class UShareTexturesLibraryItem;
struct FShareTexturesItemData;
struct FShareTexturesCategoryData;

/**
 * Widget used to display the whole library of textures 
 */
UCLASS(Abstract)
class SHARETEXTURES_API UShareTexturesLibrary : public UUserWidget
{
	GENERATED_BODY()

protected:
	/**
	 * Tile view holding all the texture items the user can choose from
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UTileView> Textures;
	/**
	 * List of available categories the user can filter the textures by
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UListView> Categories;
	/**
	 * Button opening the ShareTextures Website 
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UButton> WebsiteButton;
	/**
	 * Button opening the Patreon Website 
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UButton> PatreonButton;
	/**
	 * Button used to download the currently selected texture
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UButton> DownloadButton;
	/**
	 * Dropdown used to select the desired resolution before downloading a texture 
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UComboBoxString> ResolutionDropdown;
	/**
	 * Dropdown used to select the desired material before downloading a texture
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UComboBoxString> MaterialDropdown;
	/**
	 * Search InputField used to filter the textures
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UEditableText> SearchInput;

private:
	/**
	 * Callback executed when the download button is clicked
	 */
	UFUNCTION()
	void OnDownloadClicked();
	/**
	 * Callback executed when the search input changes
	 */
	UFUNCTION()
	void OnSearchTextChanged(const FText& Text);
	/**
	 * Callback executed when a new set of items is fetched
	 */
	void OnItemsFetched(TArray<FShareTexturesItemData> InItems);
	/**
	 * Callback executed when a new set of categories is fetched
	 */
	void OnCategoriesFetched(TArray<FShareTexturesCategoryData> InCategories);
	/**
	 * Callback executed when the texture download progress changes
	 */
	void OnDownloadProgress(TSharedPtr<IHttpRequest> Request, uint64 BytesSent, uint64 BytesReceived);
	/**
	 * Callback executed when the texture download is complete
	 */
	void OnDownloadComplete(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bSuccess);
	/**
	 * Imports all the raw files from a specific folder as assets inside Unreal
	 */
	void ImportFolder(const FString& InFolder);
	/**
	 * Callback executed when a texture item is clicked
	 */
	void OnTextureClicked(UObject* Object);
	/**
	 * Callback executed when a category item is clicked
	 */
	void OnCategoryClicked(UObject* Object);
	/**
	 * Callback executed when the launch website button is clicked
	 */
	UFUNCTION()
	void LaunchWebsite();
	/**
	 * Callback executed when the launch patreon button is clicked
	 */
	UFUNCTION()
	void LaunchPatreon();
	/**
	 * Callback executed when the resolution is changed via the dropdown
	 */
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	/**
	 * Re-evaluates the user's filtering options (search input + selected category) to ensure we only display the relevant items
	 */
	void RefreshAvailableTextures();
	/**
	 * Re-calculates the number of items in each category and updates the UI
	 */
	void RefreshCategoriesCount();
	/**
	 * Helper function to get the base material object based on the dropdown selection
	 */
	TSoftObjectPtr<UMaterial> GetSelectedBaseMaterial() const;

	//~ Begin UUserWidget Interface
	virtual void NativeConstruct() override;
	//~ End UUserWidget Interface

	/**
	 * Cache containing all the available texture items - used for filtering
	 */
	UPROPERTY(Transient)
	TArray<UShareTexturesItem*> AllTextureItems;
	/**
	 * Download task used to display the download progress to the end-user
	 */
	TUniquePtr<FSlowTask> DownloadTask;
};