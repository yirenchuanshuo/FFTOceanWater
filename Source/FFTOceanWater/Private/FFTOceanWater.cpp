// Copyright Epic Games, Inc. All Rights Reserved.

#include "FFTOceanWater.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FFFTOceanWaterModule"

void FFFTOceanWaterModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FString ShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("FFTOceanWater"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/FFTOceanWater"), ShaderDir);
}

void FFFTOceanWaterModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FFFTOceanWaterModule, FFTOceanWater)