#pragma once
#include "OceanRenderExportDataPass.h"
#include "OceanRenderHZeroPass.h"
#include "OceanRenderFrequencySpectrumPass.h"
#include "OceanRenderIFFTPass.h"

class UTextureRenderTarget2DArray;

class OceanRender
{
public:
	OceanRender();

	void Draw(FRHICommandListImmediate& RHICommandList,FOceanRenderData& SetupData,UOceanDataComponent& OceanDataComponent,
		FTextureRenderTargetResource* DebugRenderTargetRHITexture,FTextureRenderTargetResource* DebugRenderTargetRHITexture2);
	
	void Dispatch(FOceanRenderData& SetupData,UOceanDataComponent& OceanDataComponent,
		FTextureRenderTargetResource* DebugRenderTargetRHITexture = nullptr,FTextureRenderTargetResource* DebugRenderTargetRHITexture2 = nullptr);
	
	TUniquePtr<OceanRenderHZeroPass> OceanHZeroPass;
	TUniquePtr<OceanRenderFrequencySpectrumPass> FrequencyPass;
	TUniquePtr<OceanRenderIFFTPass> IFFTPass;
	TUniquePtr<OceanRenderExportDataPass> ExportDataPass;

private:
	
};
