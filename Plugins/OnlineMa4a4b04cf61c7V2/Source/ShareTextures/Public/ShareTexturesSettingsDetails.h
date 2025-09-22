// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include <IDetailCustomization.h>

/**
 * Detail Customization for the ShareTextures settings so we can display a button to clear the cache.
 */
class FShareTexturesSettingsDetails : public IDetailCustomization
{
public:
	/**
	 * Creates a new instance of this details customization
	 */
	static TSharedRef<IDetailCustomization> MakeInstance();

private:
	//~ Begin IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	//~ End IDetailCustomization interface

	/**
	 * Callback executed when the user clicks the button
	 */
	FReply OnClearCacheClicked();
	/**
	 * Callback executed to determine if it makes sense to press the button
	 */
	bool CanClearCache() const;
};