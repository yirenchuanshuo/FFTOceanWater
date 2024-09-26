#pragma once
#include "OceanFFTCommonData.h"

class UOceanDataComponent;
class FOceanBasicUniformBufferData;
class OceanRenderFrequencySpectrumPass
{
public:
	OceanRenderFrequencySpectrumPass();
	
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
