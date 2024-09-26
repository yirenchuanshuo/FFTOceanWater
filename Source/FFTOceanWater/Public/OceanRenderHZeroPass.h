#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "OceanFFTCommonData.h"
#include "Engine/World.h"

class FOceanBasicUniformBufferData;
class UOceanDataComponent;

class OceanRenderHZeroPass
{
public:
	OceanRenderHZeroPass();
	
	void Draw(FRHICommandListImmediate& RHICommandList,const FOceanRenderHZeroPassData& SetupData, const UOceanDataComponent& OceanDataComponent);
	
	FRDGTextureRef SpectrumTexture;
	FRDGTextureUAVRef SpectrumTextureUAV;

	TRefCountPtr<IPooledRenderTarget> OutputRT;
private:
	
	
};
