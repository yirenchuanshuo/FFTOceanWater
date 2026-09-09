#pragma once
#include "OceanFFTCommonData.h"

class UOceanDataComponent;
class FRDGBuilder;

class OceanRenderIFFTPass
{
public:
	OceanRenderIFFTPass();

	// Add both IFFT passes (row + column) into an existing GraphBuilder.
	// Inputs are the RDG textures produced by the FrequencySpectrum pass (same graph).
	void AddPass(FRDGBuilder& GraphBuilder, const FOceanRenderIFFTPassData& SetupData, const UOceanDataComponent& OceanDataComponent,
		FRDGTextureRef FrequencySpectrumXYInput, FRDGTextureRef FrequencySpectrumZInput);

	void Draw(FRHICommandListImmediate& RHICommandList,const FOceanRenderIFFTPassData& SetupData, const UOceanDataComponent& OceanDataComponent);
	
	FRDGTextureRef IFFTXYTexture;
	FRDGTextureRef IFFTZTexture;
	FRDGTextureUAVRef IFFTXYTextureUAV;
	FRDGTextureUAVRef IFFTZTextureUAV;

	FRDGTextureRef DisplacementTexture;
	FRDGTextureUAVRef DisplacementTextureUAV;

	TRefCountPtr<IPooledRenderTarget> OutputRTDisplacement;
	
private:
	
};
