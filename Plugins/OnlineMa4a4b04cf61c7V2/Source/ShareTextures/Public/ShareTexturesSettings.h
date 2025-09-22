// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include "ShareTexturesSettings.generated.h"

class UEditorUtilityWidgetBlueprint;

/**
 * Structure holding the data used to convert map types to material suffixes and parameter names
 */
USTRUCT()
struct FShareTexturesRenameEntry
{
	GENERATED_BODY()

	/**
	 * e.g.: "Basecolor" 
	 */
	UPROPERTY(EditAnywhere, Category = "ShareTextures")
	FString MapType;
	/**
	 * e.g.: "_D"
	 */
	UPROPERTY(EditAnywhere, Category = "ShareTextures")
	FString MaterialSuffix;
	/**
	 * e.g.: "Diffuse Map"
	 */
	UPROPERTY(EditAnywhere, Category = "ShareTextures")
	FString ParameterName;
};

/**
 * Configurable Project Settings for the ShareTextures plugin
 */
UCLASS(config=ShareTextures, meta=(DisplayName="One Click Materials"))
class UShareTexturesSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UShareTexturesSettings();
	/**
	 * Helper function to get the material suffix for a given map type
	 */
	FString GetMaterialSuffix(const FString& InMapType) const;
	/**
	 * Helper function to get the parameter name for a given map type
	 */
	FString GetParameterName(const FString& InSuffix) const;
	/**
	 * User-Configurable location where we want to store the downloaded textures & preview images
	 */
	UPROPERTY(EditAnywhere, Config, Category = "ShareTextures")
	FString CacheLocation = TEXT("");
	/**
	 * Allowed materials to be used as based when creating new materials from the library
	 */
	UPROPERTY(EditAnywhere, Config, Category = "ShareTextures")
	TArray<TSoftObjectPtr<UMaterial>> BaseMaterials;
	/**
	 * Utility widget we want to spawn and display inside the ShareTextures tab
	 */
	UPROPERTY(EditAnywhere, Config, Category = "ShareTextures")
	TSoftObjectPtr<UEditorUtilityWidgetBlueprint> TabWidgetBlueprint;
	/**
	 * All the known possibilities for renaming map types to material suffixes and parameter names
	 */
	UPROPERTY(EditAnywhere, Config, Category = "ShareTextures")
	TArray<FShareTexturesRenameEntry> RenameEntries;
	/**
	 * Score required for an item to be considered a match for a search query 
	 */
	UPROPERTY(EditAnywhere, Config, Category = "ShareTextures")
	float SearchMinimumMatchPercentage = 0.5f;
	/**
	 * These categories will be hidden from the end user 
	 */
	TArray<FString> HiddenCategories = {TEXT("Surface Imperfection"), TEXT("SBSAR"), TEXT("Other")};
	/**
	 * Items containing any of these strings in their name will be hidden from the end user 
	 */
	TArray<FString> HiddenItemNames = {TEXT("Set"), TEXT("Package"), TEXT("Real")};
	/**
	 * Item types we should display in the library to the end user 
	 */
	TArray<FString> VisibleTypes = {TEXT("Textures")};
	/**
	 * Maps the possible categories items have assigned to the way we want to display them
	 */
	TMap<FString, FString> AllowedDownloadCategories = {
		{TEXT("1k"), TEXT("1K")},
		{TEXT("2k"), TEXT("2K")},
		{TEXT("4k"), TEXT("4K")},
		{TEXT("1k Textures"), TEXT("1K")},
		{TEXT("2k Textures"), TEXT("2K")},
		{TEXT("4k Textures"), TEXT("4K")},
		{TEXT("1K"), TEXT("1K")},
		{TEXT("2K"), TEXT("2K")},
		{TEXT("4K"), TEXT("4K")},
		{TEXT("1K Textures"), TEXT("1K")},
		{TEXT("2K Textures"), TEXT("2K")},
		{TEXT("4K Textures"), TEXT("4K")}
	};

private:
	// ~Begin UDeveloperSettings Interface
	virtual void PostInitProperties() override;
	virtual FName GetCategoryName() const override;
	// ~End UDeveloperSettings Interface
};