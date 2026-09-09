#pragma once
#include "OceanFFTCommonData.h"

class UOceanDataComponent;
class FOceanBasicUniformBufferData;
class FRDGBuilder;

class OceanRenderFrequencySpectrumPass
{
public:
	OceanRenderFrequencySpectrumPass();

	// Add the FrequencySpectrum pass into an existing GraphBuilder.
	// HZeroTextureInput is the RDG texture representing HZero (registered externally, or produced in the same graph).
	void AddPass(FRDGBuilder& GraphBuilder, const FOceanRenderFrequencySpectrumPassData& SetupData, const UOceanDataComponent& OceanDataComponent, FRDGTextureRef HZeroTextureInput);

	void Draw(FRHICommandListImmediate& RHICommandList,const FOceanRenderFrequencySpectrumPassData& SetupData, const UOceanDataComponent& OceanDataComponent);


	FRDGTextureRef FFTXYTexture;
	FRDGTextureUAVRef FFTXYTextureUAV;
	
	FRDGTextureRef FFTZTexture;
	FRDGTextureUAVRef FFTZTextureUAV;

	FRDGTextureRef HZeroTexture;
	TRefCountPtr<IPooledRenderTarget> OutputRTXY;
	TRefCountPtr<IPooledRenderTarget> OutputRTZ;
	
private:
	
};
