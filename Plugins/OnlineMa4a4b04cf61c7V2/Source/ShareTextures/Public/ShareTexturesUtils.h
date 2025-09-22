// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

namespace ShareTexturesUtils
{

	/**
	 * Parse the name of the asset to get the resolution, asset name and map type.
	 * @note the input name can have various formats depending on when it was added to the backend
	 */
	bool ParseName(const FString& InName, FString& OutResolution, FString& OutAssetName, FString& OutMapType);
}