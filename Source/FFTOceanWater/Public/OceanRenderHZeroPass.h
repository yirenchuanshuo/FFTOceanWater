#pragma once
#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "RHIResources.h"
#include "RHICommandList.h"
#include "OceanFFTCommonData.h"
#include "Engine/World.h"

class FOceanBasicUniformBufferData;
class UOceanDataComponent;
class FRDGBuilder;

class OceanRenderHZeroPass
{
public:
	OceanRenderHZeroPass();

	// Add the HZero pass into an existing GraphBuilder. Also queues extraction into OutputRT
	// so that the result can be reused across frames.
	void AddPass(FRDGBuilder& GraphBuilder, const FOceanRenderHZeroPassData& SetupData, const UOceanDataComponent& OceanDataComponent);

	// Legacy path: builds its own GraphBuilder and executes immediately.
	void Draw(FRHICommandListImmediate& RHICommandList, const FOceanRenderHZeroPassData& SetupData, const UOceanDataComponent& OceanDataComponent);

	FRDGTextureRef SpectrumTexture;
	FRDGTextureUAVRef SpectrumTextureUAV;

	TRefCountPtr<IPooledRenderTarget> OutputRT;
private:


};
