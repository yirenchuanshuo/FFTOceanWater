#pragma once
#include "OceanFFTCommonData.h"

class UOceanDataComponent;
class OceanRenderIFFTPass
{
public:
	OceanRenderIFFTPass();

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
