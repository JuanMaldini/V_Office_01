// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include <Blueprint/IUserObjectListEntry.h>
#include <Blueprint/UserWidget.h>

#include "ShareTexturesLibraryCategory.generated.h"

class UTextBlock;

/**
 * Widget used to display a single category in the library
 */
UCLASS(Abstract)
class UShareTexturesLibraryCategory : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

protected:
	/**
	 * Text block used to display the category's title/name
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UTextBlock> Title;

	/**
	 * Text block used to display the number of items in the category
	 */
	UPROPERTY(BlueprintReadOnly, Category = "ShareTextures", meta = (BindWidget))
	TObjectPtr<UTextBlock> Count;

private:
	//~ Begin IUserObjectListEntry Interface
	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	//~ End IUserObjectListEntry Interface

	/**
	 * Callback executed to get the curent item count for this category
	 */
	UFUNCTION()
	FText GetItemCount();
};