// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesUtils.h"

#include "ShareTexturesSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FShareTexturesNameParsing, "ShareTextures.Utils.ParseName", EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter);

bool FShareTexturesNameParsing::RunTest(const FString& Parameters)
{
	auto TestAssetName = [this](const FString& InName, const FString& Resolution, const FString& AssetName, const FString& MapType)
	{
		FString OutResolution;
		FString OutAssetName;
		FString OutMapType;
		const bool bParseSuccess = ShareTexturesUtils::ParseName(InName, OutResolution, OutAssetName, OutMapType);

		TestTrue(TEXT("Parsing successful: " + InName), bParseSuccess);
		TestEqual(TEXT("Correct Resolution: " + InName), OutResolution, Resolution);
		TestEqual(TEXT("Correct AssetName: " + InName), OutAssetName, AssetName);
		TestEqual(TEXT("Correct MapType: " + InName), OutMapType, MapType);
	};

	TestAssetName(TEXT("1K-amethyst_1-ao"), TEXT("1K"), TEXT("Amethyst1"), TEXT("ao"));
	TestAssetName(TEXT("1K-amethyst_1-diffuse"), TEXT("1K"), TEXT("Amethyst1"), TEXT("diffuse"));
	TestAssetName(TEXT("1K-amethyst_1-displacement"), TEXT("1K"), TEXT("Amethyst1"), TEXT("displacement"));
	TestAssetName(TEXT("1K-amethyst_1-normal"), TEXT("1K"), TEXT("Amethyst1"), TEXT("normal"));
	TestAssetName(TEXT("1K-amethyst_1-specular"), TEXT("1K"), TEXT("Amethyst1"), TEXT("specular"));

	TestAssetName(TEXT("1K-plaster_7-diffuse"), TEXT("1K"), TEXT("Plaster7"), TEXT("diffuse"));
	TestAssetName(TEXT("2K-plaster_4-displacement"), TEXT("2K"), TEXT("Plaster4"), TEXT("displacement"));
	TestAssetName(TEXT("4K-ceramic_indian-ao"), TEXT("4K"), TEXT("CeramicIndian"), TEXT("ao"));

	TestAssetName(TEXT("1K-rock_ground_1-normal"), TEXT("1K"), TEXT("RockGround1"), TEXT("normal"));
	TestAssetName(TEXT("1K-wall_stone_10-normal"), TEXT("1K"), TEXT("WallStone10"), TEXT("normal"));
	TestAssetName(TEXT("1K-wall_stone_4-normal"), TEXT("1K"), TEXT("WallStone4"), TEXT("normal"));
	TestAssetName(TEXT("1K-wood_fine_6-normal"), TEXT("1K"), TEXT("WoodFine6"), TEXT("normal"));

	TestAssetName(TEXT("blackmarble_1_ao-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("ao"));
	TestAssetName(TEXT("blackmarble_1_diffuse-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("diffuse"));
	TestAssetName(TEXT("blackmarble_1_displacement-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("displacement"));
	TestAssetName(TEXT("blackmarble_1_normal-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("normal"));
	TestAssetName(TEXT("blackmarble_1_specular-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("specular"));

	TestAssetName(TEXT("blackmarble_1_ambientocclusion-1K"), TEXT("1K"), TEXT("Blackmarble1"), TEXT("ambientocclusion"));
	TestAssetName(TEXT("blackmarble_1_ambientocclusion-2K"), TEXT("2K"), TEXT("Blackmarble1"), TEXT("ambientocclusion"));
	TestAssetName(TEXT("blackmarble_1_ambientocclusion-4K"), TEXT("4K"), TEXT("Blackmarble1"), TEXT("ambientocclusion"));

	TestAssetName(TEXT("fossilgrey_1_ambientocclusion-1K"), TEXT("1K"), TEXT("Fossilgrey1"), TEXT("ambientocclusion"));
	TestAssetName(TEXT("camouflage_9_basecolor-1K"), TEXT("1K"), TEXT("Camouflage9"), TEXT("basecolor"));

	TestAssetName(TEXT("1K-wallpaper_40_roughness"), TEXT("1K"), TEXT("Wallpaper40"), TEXT("roughness"));

	TestAssetName(TEXT("1K_metalicus_selection_2_polishedmarble_specular"), TEXT("1K"), TEXT("MetalicusSelection2Polishedmarble"), TEXT("specular"));

	TestAssetName(TEXT("fabric_202_Albedo-1K"), TEXT("1K"), TEXT("Fabric202"), TEXT("albedo"));

	return true;
}

/*
 * Helper function to find a string inside an array by performing insensitive comparison.
 */
bool InsensitiveContains(const TArray<FString> InArray, const FString& InElement)
{
	for (const FString& ArrayElement : InArray)
	{
		if (ArrayElement.Equals(InElement, ESearchCase::IgnoreCase))
		{
			return true;
		}
	}

	return false;
}

bool ShareTexturesUtils::ParseName(const FString& InName, FString& OutResolution, FString& OutAssetName, FString& OutMapType)
{
	const TArray<FString> PossibleResolutions = {TEXT("1K"), TEXT("2K"), TEXT("4K")};

	TArray<FString> PossibleTypemaps;
	for (const FShareTexturesRenameEntry& Entry : GetDefault<UShareTexturesSettings>()->RenameEntries)
	{
		PossibleTypemaps.Add(Entry.MapType);
	}

	// Sometimes the names are split with dashes, sometimes with underscores
	// We are converting all to `-` to ensure we can split the array correctly
	FString ParsedName = InName;
	ParsedName.ReplaceInline(TEXT("-"), TEXT("_"));
	ParsedName.RemoveSpacesInline();

	TArray<FString> NameParts;
	ParsedName.ParseIntoArray(NameParts, TEXT("_"));

	for (auto PartIt = NameParts.CreateIterator(); PartIt; ++PartIt)
	{
		const FString& Part = *PartIt;
		if (InsensitiveContains(PossibleResolutions, Part))
		{
			OutResolution = Part.ToUpper();
			PartIt.RemoveCurrent();
		}
		if (InsensitiveContains(PossibleTypemaps, Part))
		{
			OutMapType = Part.ToLower();
			PartIt.RemoveCurrent();
		}
	}

	// The asset name is the non-resolution and non-map type parts combined
	for (FString RemainingPart : NameParts)
	{
		RemainingPart.GetCharArray()[0] = TChar<TCHAR>::ToUpper(RemainingPart.GetCharArray()[0]);
		OutAssetName.Append(RemainingPart);
	}

	return !OutResolution.IsEmpty() && !OutMapType.IsEmpty() && !OutAssetName.IsEmpty();
}