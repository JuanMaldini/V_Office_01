// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesLibrary.h"

#include <AssetToolsModule.h>
#include <Algo/Count.h>
#include <BlueprintMaterialTextureNodesBPLibrary.h>
#include <HttpModule.h>
#include <Algo/LevenshteinDistance.h>
#include <Components/Button.h>
#include <Components/ComboBoxString.h>
#include <Components/EditableText.h>
#include <Components/TileView.h>
#include <FileUtilities/ZipArchiveReader.h>
#include <Interfaces/IHttpResponse.h>

#include "ShareTexturesData.h"
#include "ShareTexturesLibraryCategory.h"
#include "ShareTexturesLog.h"
#include "ShareTexturesSettings.h"
#include "ShareTexturesSubsystem.h"
#include "ShareTexturesUtils.h"
#include "Algo/AnyOf.h"

#define LOCTEXT_NAMESPACE "ShareTextures"

namespace
{
	float GetScore(TSharedPtr<FShareTexturesItemData> ItemData, const FString& InputSearch)
	{
		if(ItemData->Title.Equals(InputSearch, ESearchCase::IgnoreCase))
		{
			// Perfect match
			return 10.0f;
		}
		if(ItemData->Title.Contains(InputSearch, ESearchCase::IgnoreCase))
		{
			// Potential match in progress
			return 8.0f;
		}

		TArray<FString> SearchTerms;
		InputSearch.ParseIntoArray(SearchTerms, TEXT(" "));
		SearchTerms.Sort();
		
		float Score = 0.0f;
		for(const FShareTexturesTag& Tag : ItemData->Tags)
		{
			if(SearchTerms.Contains(Tag.Text))
			{
				Score += 1.0f;
			}
		}

		TArray<FString> TitleTerms;
		ItemData->Title.ParseIntoArray(TitleTerms, TEXT(" "));
		TitleTerms.Sort();

		for(const FString& TitleTerm : TitleTerms)
		{
			if(SearchTerms.Contains(TitleTerm))
			{
				Score += 1.0f;
			}
		}

		return Score;
	}
}

void UShareTexturesLibrary::OnItemsFetched(TArray<FShareTexturesItemData> InItems)
{
	if (!ensure(Textures))
	{
		return;
	}

	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();

	AllTextureItems.Empty();

	for (const FShareTexturesItemData& Item : InItems)
	{
		if (!Settings->VisibleTypes.Contains(Item.ItemType.Name))
		{
			continue;
		}

		if (Settings->HiddenCategories.Contains(Item.Category.Name))
		{
			continue;
		}

		if(Item.PublishDate > FDateTime::Now())
		{
			continue;
		}

		TArray<FString> NameParts;
		Item.Title.ParseIntoArray(NameParts, TEXT(" "));
		const bool bShouldHideByName = Algo::AnyOf(NameParts,
		                                           [Settings](const FString& Part)
		                                           {
			                                           return Settings->HiddenItemNames.Contains(Part);
		                                           });
		if (bShouldHideByName)
		{
			continue;
		}

		TSharedPtr<FShareTexturesItemData> SharedItem = MakeShared<FShareTexturesItemData>(Item);
		for (auto DownloadLinkIt = SharedItem->DownloadLinks.CreateIterator(); DownloadLinkIt; ++DownloadLinkIt)
		{
			if (const FString* FoundTitle = Settings->AllowedDownloadCategories.Find(DownloadLinkIt->Title))
			{
				DownloadLinkIt->Title = *FoundTitle;
			}
			else
			{
				DownloadLinkIt.RemoveCurrent();
			}
		}

		UShareTexturesItem* TextureItem = NewObject<UShareTexturesItem>();
		TextureItem->ItemData = SharedItem;
		TextureItem->CurrentResolution = ResolutionDropdown->GetSelectedOption();

		AllTextureItems.Add(TextureItem);
	}

	RefreshAvailableTextures();
	RefreshCategoriesCount();
}

void UShareTexturesLibrary::OnCategoriesFetched(TArray<FShareTexturesCategoryData> InCategories)
{
	if (!ensure(Categories))
	{
		return;
	}

	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();

	Categories->ClearListItems();

	for (const FShareTexturesCategoryData& Category : InCategories)
	{
		if (Settings->HiddenCategories.Contains(Category.Name))
		{
			continue;
		}

		TSharedPtr<FShareTexturesCategoryData> SharedCategory = MakeShared<FShareTexturesCategoryData>(Category);

		UShareTexturesCategory* TextureCategory = NewObject<UShareTexturesCategory>();
		TextureCategory->CategoryData = SharedCategory;

		Categories->AddItem(TextureCategory);
	}

	RefreshCategoriesCount();
}

void UShareTexturesLibrary::NativeConstruct()
{
	Super::NativeConstruct();

	constexpr int NewHttpRequestLimit = 10000;
	UE_LOG(LogShareTextures, Display, TEXT("Increasing RunningThreadedRequestLimitEditor to %s"), *LexToString(NewHttpRequestLimit));
	GConfig->SetInt(TEXT("HTTP.HttpThread"), TEXT("RunningThreadedRequestLimitEditor"), NewHttpRequestLimit, GEditorIni);

	FHttpModule::Get().UpdateConfigs();

	if (ensure(DownloadButton))
	{
		DownloadButton->OnClicked.AddDynamic(this, &UShareTexturesLibrary::OnDownloadClicked);
	}

	if (ensure(SearchInput))
	{
		SearchInput->OnTextChanged.AddDynamic(this, &UShareTexturesLibrary::OnSearchTextChanged);
	}

	if (ensure(ResolutionDropdown))
	{
		// Manually adding a resolution so we have a default selection
		ResolutionDropdown->AddOption(TEXT("1K"));
		ResolutionDropdown->SetSelectedOption(TEXT("1K"));

		ResolutionDropdown->OnSelectionChanged.AddDynamic(this, &UShareTexturesLibrary::OnResolutionChanged);
	}

	if (ensure(Categories))
	{
		Categories->OnItemSelectionChanged().AddUObject(this, &UShareTexturesLibrary::OnCategoryClicked);
		Categories->SetSelectionMode(ESelectionMode::SingleToggle);
	}

	if (ensure(Textures))
	{
		Textures->OnItemSelectionChanged().AddUObject(this, &UShareTexturesLibrary::OnTextureClicked);
		Textures->SetSelectionMode(ESelectionMode::SingleToggle);
	}

	if (ensure(WebsiteButton))
	{
		WebsiteButton->OnClicked.AddDynamic(this, &UShareTexturesLibrary::LaunchWebsite);
	}

	if (ensure(PatreonButton))
	{
		PatreonButton->OnClicked.AddDynamic(this, &UShareTexturesLibrary::LaunchPatreon);
	}

	UShareTexturesSubsystem* Subsystem = GEditor->GetEditorSubsystem<UShareTexturesSubsystem>();

	TResponseDataDelegate<FShareTexturesItemData> OnItemsFetched = TResponseDataDelegate<FShareTexturesItemData>::CreateUObject(this, &UShareTexturesLibrary::OnItemsFetched);
	Subsystem->SendFetchItemsRequest(OnItemsFetched);

	TResponseDataDelegate<FShareTexturesCategoryData> OnCategoriesFetched = TResponseDataDelegate<FShareTexturesCategoryData>::CreateUObject(this, &UShareTexturesLibrary::OnCategoriesFetched);
	Subsystem->SendFetchCategoriesRequest(OnCategoriesFetched);

	if (ensure(MaterialDropdown))
	{
		const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
		for (const TSoftObjectPtr<UMaterial>& Material : Settings->BaseMaterials)
		{
			MaterialDropdown->AddOption(Material.GetAssetName());
		}

		MaterialDropdown->SetSelectedIndex(0);
	}
}

TSoftObjectPtr<UMaterial> UShareTexturesLibrary::GetSelectedBaseMaterial() const
{
	if (!ensure(MaterialDropdown))
	{
		return {};
	}

	const FString SelectedMaterial = MaterialDropdown->GetSelectedOption();
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	const TSoftObjectPtr<UMaterial>* BaseMaterial = Settings->BaseMaterials.FindByPredicate([SelectedMaterial](const TSoftObjectPtr<UMaterial>& Item)
	{
		return Item.GetAssetName() == SelectedMaterial;
	});
	if (BaseMaterial)
	{
		return *BaseMaterial;
	}

	return {};
}

void UShareTexturesLibrary::OnDownloadClicked()
{
	UShareTexturesItem* SelectedItem = Textures->GetSelectedItem<UShareTexturesItem>();
	if (!SelectedItem)
	{
		return;
	}

	if (SelectedItem->IsCached())
	{
		ImportFolder(SelectedItem->GetCacheLocation());
	}
	else
	{
		const FString DownloadLink = SelectedItem->GetDownloadLink();
		const TSoftObjectPtr<UMaterial> BaseMaterial = GetSelectedBaseMaterial();

		if (!BaseMaterial || DownloadLink.IsEmpty())
		{
			return;
		}

		FHttpRequestRef DownloadRequest = FHttpModule::Get().CreateRequest();
		DownloadRequest->SetURL(DownloadLink);
		DownloadRequest->SetVerb(TEXT("GET"));
		DownloadRequest->OnRequestProgress64().BindUObject(this, &UShareTexturesLibrary::OnDownloadProgress);
		DownloadRequest->OnProcessRequestComplete().BindUObject(this, &UShareTexturesLibrary::OnDownloadComplete);
		DownloadRequest->ProcessRequest();

		DownloadTask = MakeUnique<FSlowTask>(1.0f, LOCTEXT("DownloadingTexture", "Downloading texture..."));
		DownloadTask->Initialize();
		DownloadTask->MakeDialog(false, true);
	}
}

void UShareTexturesLibrary::OnSearchTextChanged(const FText& Text)
{
	RefreshAvailableTextures();
}

void UShareTexturesLibrary::OnDownloadProgress(TSharedPtr<IHttpRequest> Request, uint64 BytesSent, uint64 BytesReceived)
{
	const FText Message = FText::Format(LOCTEXT("Downloading", "Downloading textures - {0}"), FText::AsMemory(BytesReceived));
	DownloadTask->EnterProgressFrame(0.0f, Message);
}

void UShareTexturesLibrary::OnDownloadComplete(TSharedPtr<IHttpRequest> HttpRequest, TSharedPtr<IHttpResponse> HttpResponse, bool bSuccess)
{
	DownloadTask->Destroy();
	DownloadTask.Reset();

	UShareTexturesItem* SelectedItem = Textures->GetSelectedItem<UShareTexturesItem>();
	const FString ArchiveFile = SelectedItem->GetCacheLocation() / "raw.zip";
	const FString ImportFolderPath = SelectedItem->GetCacheLocation();

	FFileHelper::SaveArrayToFile(HttpResponse->GetContent(), *ArchiveFile);

	UE_LOG(LogShareTextures, VeryVerbose, TEXT("File saved to: %s from link: %s"), *ArchiveFile, *HttpRequest->GetURL())

	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	IFileHandle* ZipFile = PlatformFile.OpenRead(*ArchiveFile);
	TSharedPtr<FZipArchiveReader> ZipReader = MakeShared<FZipArchiveReader>(ZipFile);

	for (const FString& EmbeddedFileName : ZipReader->GetFileNames())
	{
		if (FPaths::GetExtension(EmbeddedFileName).IsEmpty())
		{
			// We don't care about folders
			continue;
		}

		TArray<uint8> Contents;
		if (!ZipReader->TryReadFile(EmbeddedFileName, Contents))
		{
			continue;
		}

		const FString FileNameClean = FPaths::GetBaseFilename(EmbeddedFileName);
		const FString FileExtension = FPaths::GetExtension(EmbeddedFileName, true);

		FString OutResolution;
		FString OutAssetName;
		FString OutMapType;
		const bool bParseSuccess = ShareTexturesUtils::ParseName(FileNameClean, OutResolution, OutAssetName, OutMapType);

		if (!bParseSuccess)
		{
			UE_LOG(LogShareTextures, Error, TEXT("Failed to parse file name: %s"), *FileNameClean);
			continue;
		}

		const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
		const FString TextureSuffix = Settings->GetMaterialSuffix(OutMapType);
		if (TextureSuffix.IsEmpty())
		{
			UE_LOG(LogShareTextures, Warning, TEXT("Couldn't find suffix based on map type: %s"), *OutMapType);
		}

		const FString FinalFileName = ImportFolderPath / TEXT("T_") + OutAssetName + TEXT("-") + OutResolution + TextureSuffix + FileExtension;

		FFileHelper::SaveArrayToFile(Contents, *FinalFileName);

		UE_LOG(LogShareTextures, VeryVerbose, TEXT("Unarching file to: %s from archive: %s"), *FinalFileName, *ArchiveFile);
	}

	ZipReader.Reset();
	IFileManager::Get().Delete(*ArchiveFile);

	ImportFolder(ImportFolderPath);
}

void UShareTexturesLibrary::ImportFolder(const FString& InFolder)
{
	UShareTexturesItem* SelectedItem = Textures->GetSelectedItem<UShareTexturesItem>();
	const FString Title = SelectedItem->GetTitle();
	const FString ImportFolder = SelectedItem->GetImportPath();

	UMaterial* DefaultMaterial = GetSelectedBaseMaterial().Get();
	const FString MaterialName = ImportFolder / TEXT("MI_") + Title;
	UMaterialInstanceConstant* NewMaterial = UBlueprintMaterialTextureNodesBPLibrary::CreateMIC_EditorOnly(DefaultMaterial, MaterialName);

	TArray<FString> FilesInDirectory;
	IFileManager::Get().FindFilesRecursive(FilesInDirectory, *InFolder, TEXT("*.*"), true, false);

	for (TArray<FString>::TConstIterator FileIter(FilesInDirectory); FileIter; ++FileIter)
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools");
		UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
		ImportData->bReplaceExisting = true;
		ImportData->Filenames = {*FileIter};
		ImportData->DestinationPath = ImportFolder / TEXT("Textures");
		TArray<UObject*> ImportedObjects = AssetToolsModule.Get().ImportAssetsAutomated(ImportData);

		for (UObject* ImportedObject : ImportedObjects)
		{
			const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();

			const FString TextureSuffix = ImportedObject->GetName().Right(2);
			const FString ParameterName = Settings->GetParameterName(TextureSuffix);
			UTexture2D* ImportedTexture = Cast<UTexture2D>(ImportedObject);
			if (ImportedTexture && !ParameterName.IsEmpty())
			{
				UBlueprintMaterialTextureNodesBPLibrary::SetMICTextureParam_EditorOnly(NewMaterial, ParameterName, ImportedTexture);
			}
		}
	}
}

void UShareTexturesLibrary::OnTextureClicked(UObject* Object)
{
	UShareTexturesItem* SelectedItem = Textures ? Textures->GetSelectedItem<UShareTexturesItem>() : nullptr;
	const UShareTexturesSettings* Settings = GetDefault<UShareTexturesSettings>();
	if (!Settings || !SelectedItem || !SelectedItem->ItemData)
	{
		return;
	}

	if (!ensure(ResolutionDropdown))
	{
		return;
	}

	const FString InitialResolution = ResolutionDropdown->GetSelectedOption();
	ResolutionDropdown->ClearOptions();

	for (const FShareTexturesDownloadLink& DownloadLink : SelectedItem->ItemData->DownloadLinks)
	{
		const FString ResolutionTitle = DownloadLink.Title;
		ResolutionDropdown->AddOption(DownloadLink.Title);
	}

	const int IndexToSelect = FMath::Max(ResolutionDropdown->FindOptionIndex(InitialResolution), 0);
	ResolutionDropdown->SetSelectedIndex(IndexToSelect);
}

void UShareTexturesLibrary::OnCategoryClicked(UObject* Object)
{
	RefreshAvailableTextures();
}

void UShareTexturesLibrary::LaunchWebsite()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.sharetextures.com"), nullptr, nullptr);
}

void UShareTexturesLibrary::LaunchPatreon()
{
	FPlatformProcess::LaunchURL(TEXT("https://www.patreon.com/sharetextures"), nullptr, nullptr);
}

void UShareTexturesLibrary::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	for (UShareTexturesItem* Texture : AllTextureItems)
	{
		Texture->CurrentResolution = SelectedItem;
	}
}

void UShareTexturesLibrary::RefreshAvailableTextures()
{
	const FString CurrentSearchText = SearchInput->GetText().ToString();

	TArray<TPair<UShareTexturesItem*, float>> ScoreMap;

	for (UShareTexturesItem* Item : AllTextureItems)
	{
		const float TotalScore = !CurrentSearchText.IsEmpty() ? GetScore(Item->ItemData, CurrentSearchText) : 1.0f;
		ScoreMap.Emplace(Item, TotalScore);
	}

	ScoreMap.Sort([](TPair<UShareTexturesItem*, float> A, TPair<UShareTexturesItem*, float> B)
	{
		if(A.Value != B.Value)
		{
			return A.Value > B.Value;
		}

		return A.Key->ItemData->PublishDate > B.Key->ItemData->PublishDate;
	});

	Textures->ClearListItems();
	for (const TPair<UShareTexturesItem*, float> FilteredItem : ScoreMap)
	{
		UShareTexturesCategory* SelectedCategory = Categories->GetSelectedItem<UShareTexturesCategory>();
		const bool bScorePass = FilteredItem.Value > GetDefault<UShareTexturesSettings>()->SearchMinimumMatchPercentage;
		const bool bCategoryPass = SelectedCategory ? FilteredItem.Key->ItemData->Category.Name == SelectedCategory->CategoryData->Name : true;
		if (bScorePass && bCategoryPass)
		{
			Textures->AddItem(FilteredItem.Key);
		}
	}
}

void UShareTexturesLibrary::RefreshCategoriesCount()
{
	for (UObject* CategoryItem : Categories->GetListItems())
	{
		UShareTexturesCategory* Category = CastChecked<UShareTexturesCategory>(CategoryItem);
		Category->ItemCount = Algo::CountIf(AllTextureItems,
		                                    [Category](UShareTexturesItem* Item)
		                                    {
			                                    return Item->ItemData->Category.Name == Category->CategoryData->Name;
		                                    });
	}
}

#undef LOCTEXT_NAMESPACE