// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

/**
 * Declares the ShareTextures extension visual style. 
 */
class FShareTexturesStyle
{
public:
	/**
	 * Initializes the SlateStyle
	 */
	static void Initialize();
	/**
	 * Deinitializes the SlateStyle
	 */
	static void Shutdown();

	/**
	 * Singleton getter for the SlateStyle class
	 */
	static TSharedPtr<class ISlateStyle> Get();
	/**
	 * Convince getter for name of the current SlateStyle
	 */
	static FName GetStyleSetName();

private:
	/**
	 * Singleton instance of the SlateStyle
	 */
	static TSharedPtr<class FSlateStyleSet> StyleSet;
};