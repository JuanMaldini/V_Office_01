// Copyright ShareTextures 2024. All Rights Reserved.

#pragma once

#include <Modules/ModuleManager.h>

/**
 * Editor module responsible for integrating the ShareTextures functionality and give the developers access to the widget
 */
class FShareTexturesModule final : public IModuleInterface
{
	//~Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~End IModuleInterface Interface

	/**
	 * Callback executed the register the ShareTextures tab in the toolbar
	 */
	void RegisterMenus();
	/**
	 * Callback executed the spawn the ShareTextures widget utility as a nomad tab
	 */
	void OpenShareTexturesTab();
};
