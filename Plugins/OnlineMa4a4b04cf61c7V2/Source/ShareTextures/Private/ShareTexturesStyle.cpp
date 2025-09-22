// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesStyle.h"

#include <Interfaces/IPluginManager.h>
#include <Styling/ISlateStyle.h>
#include <Styling/SlateStyle.h>
#include <Styling/SlateStyleRegistry.h>

TSharedPtr<FSlateStyleSet> FShareTexturesStyle::StyleSet = nullptr;

TSharedPtr<ISlateStyle> FShareTexturesStyle::Get()
{
	return StyleSet;
}

FName FShareTexturesStyle::GetStyleSetName()
{
	static FName ShareTexturesStyleName(TEXT("ShareTexturesStyle"));
	return ShareTexturesStyleName;
}

void FShareTexturesStyle::Initialize()
{
	// Only register once
	if (StyleSet.IsValid())
	{
		return;
	}

	StyleSet = MakeShared<FSlateStyleSet>(GetStyleSetName());
	FString Root = IPluginManager::Get().FindPlugin(TEXT("OneClickMaterials"))->GetBaseDir() / TEXT("Resources");

	StyleSet->Set("ShareTextures.ToolbarIcon", new FSlateImageBrush(FName(*(Root / "ToolbarIcon.png")), FVector2D(256, 256)));

	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Get());
};


void FShareTexturesStyle::Shutdown()
{
	if (StyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Get());
		ensure(StyleSet.IsUnique());
		StyleSet.Reset();
	}
}