// Copyright ShareTextures 2024. All Rights Reserved.

#include "ShareTexturesSettings.h"

#include <EditorUtilityWidgetBlueprint.h>

UShareTexturesSettings::UShareTexturesSettings()
{
	static ConstructorHelpers::FObjectFinder<UEditorUtilityWidgetBlueprint> WidgetRef(TEXT("/OneClickMaterials/ShareTexturesTab"));
	check(WidgetRef.Succeeded());
	TabWidgetBlueprint = WidgetRef.Object;

	static ConstructorHelpers::FObjectFinder<UMaterial> BaseMaterialRef(TEXT("/OneClickMaterials/MasterMaterials/M_BasicMaster"));
	check(BaseMaterialRef.Succeeded());
	BaseMaterials.Add(BaseMaterialRef.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> NaniteMaterialRef(TEXT("/OneClickMaterials/MasterMaterials/M_NaniteTessellation"));
	check(NaniteMaterialRef.Succeeded());
	BaseMaterials.Add(NaniteMaterialRef.Object);

	static ConstructorHelpers::FObjectFinder<UMaterial> ParallaxMaterialRef(TEXT("/OneClickMaterials/MasterMaterials/M_ParallaxOcclusion"));
	check(ParallaxMaterialRef.Succeeded());
	BaseMaterials.Add(ParallaxMaterialRef.Object);
}

FString UShareTexturesSettings::GetMaterialSuffix(const FString& InMapType) const
{
	for (const FShareTexturesRenameEntry& RenameEntry : RenameEntries)
	{
		if (InMapType == RenameEntry.MapType)
		{
			return RenameEntry.MaterialSuffix;
		}
	}

	return TEXT("");
}

FString UShareTexturesSettings::GetParameterName(const FString& InSuffix) const
{
	for (const FShareTexturesRenameEntry& RenameEntry : RenameEntries)
	{
		if (InSuffix == RenameEntry.MaterialSuffix)
		{
			return RenameEntry.ParameterName;
		}
	}

	return TEXT("");
}

void UShareTexturesSettings::PostInitProperties()
{
	Super::PostInitProperties();

	if (CacheLocation.IsEmpty())
	{
		//TODO: This should be UserPerProject config rather than a general config
		CacheLocation = FString(FPlatformProcess::UserTempDir()) / TEXT("ShareTexturesUnrealEngine");
	}

	RenameEntries.Add({TEXT("Normal"), TEXT("_N"), TEXT("Normal Map")});
	RenameEntries.Add({TEXT("Nrm"), TEXT("_N"), TEXT("Normal Map")});
	RenameEntries.Add({TEXT("Nrml"), TEXT("_N"), TEXT("Normal Map")});

	RenameEntries.Add({TEXT("Height"), TEXT("_H"), TEXT("Height Map")});
	RenameEntries.Add({TEXT("Displacement"), TEXT("_H"), TEXT("Height Map")});
	RenameEntries.Add({TEXT("Disp"), TEXT("_H"), TEXT("Height Map")});

	RenameEntries.Add({TEXT("Basecolor"), TEXT("_D"), TEXT("Diffuse Map")});
	RenameEntries.Add({TEXT("Diffuse"), TEXT("_D"), TEXT("Diffuse Map")});
	RenameEntries.Add({TEXT("Color"), TEXT("_D"), TEXT("Diffuse Map")});
	RenameEntries.Add({TEXT("Albedo"), TEXT("_D"), TEXT("Diffuse Map")});

	RenameEntries.Add({TEXT("Roughness"), TEXT("_R"), TEXT("Roughness Map")});
	RenameEntries.Add({TEXT("Rough"), TEXT("_R"), TEXT("Roughness Map")});

	RenameEntries.Add({TEXT("Glossiness"), TEXT("_R"), TEXT("Roughness Map")});
	RenameEntries.Add({TEXT("Gloss"), TEXT("_R"), TEXT("Roughness Map")});

	RenameEntries.Add({TEXT("Specular"), TEXT("_S"), TEXT("Specular Map")});
	RenameEntries.Add({TEXT("spec"), TEXT("_S"), TEXT("Specular Map")});

	RenameEntries.Add({TEXT("AmbientOcclusion"), TEXT("_O"), TEXT("AO Map")});
	RenameEntries.Add({TEXT("ao"), TEXT("_O"), TEXT("AO Map")});
	RenameEntries.Add({TEXT("Occlusion"), TEXT("_O"), TEXT("AO Map")});

	RenameEntries.Add({TEXT("Emissive"), TEXT("_E"), TEXT("Emissive Map")});

	RenameEntries.Add({TEXT("Opacity"), TEXT("_A"), TEXT("Opacity Map")});
	RenameEntries.Add({TEXT("Alpha"), TEXT("_A"), TEXT("Opacity Map")});

	RenameEntries.Add({TEXT("Metallic"), TEXT("_M"), TEXT("Metallic Map")});
	RenameEntries.Add({TEXT("Metal"), TEXT("_M"), TEXT("Metallic Map")});
}

FName UShareTexturesSettings::GetCategoryName() const
{
	return FName(TEXT("Plugins"));
}