#pragma once
#include "OceanFFTCommonData.h"

class UOceanDataComponent;
class FRDGBuilder;

class OceanRenderExportDataPass
{
public:
	// Add the export-data passes (vertex + pixel export) into an existing GraphBuilder.
	// DisplacementInput is the RDG texture produced by the IFFT pass in the same graph.
	// DisplacementPreviousExternal / FoamPreviousExternal are RHI textures cached from the previous frame.
	void AddPass(FRDGBuilder& GraphBuilder, const FOceanRenderExportDataPassData& SetupData, const UOceanDataComponent& OceanDataComponent,
		FRDGTextureRef DisplacementInput);

	void Draw(FRHICommandListImmediate& RHICommandList,const FOceanRenderExportDataPassData& SetupData , const UOceanDataComponent& OceanDataComponent);
	
	FRDGTextureRef DisplacementTextureOutput;
	FRDGTextureUAVRef DisplacementTextureOutputUAV;

	FRDGTextureRef DisplacementTextureOutput_Previous;
	FRDGTextureUAVRef DisplacementTextureOutput_PreviousUAV;

	FRDGTextureRef PixelData_A;
	FRDGTextureUAVRef PixelDataUAV_A;
	
	FRDGTextureRef PixelData_B;
	FRDGTextureUAVRef PixelDataUAV_B;
	
	TRefCountPtr<IPooledRenderTarget> PixelData_A_OutPut;
	TRefCountPtr<IPooledRenderTarget> PixelData_B_OutPut;

	
	FRDGTextureRef PixelAttributeA_00;
	FRDGTextureUAVRef PixelAttributeA_UAV_00;
	FRDGTextureRef PixelAttributeA_01;
	FRDGTextureUAVRef PixelAttributeA_UAV_01;
	FRDGTextureRef PixelAttributeA_02;
	FRDGTextureUAVRef PixelAttributeA_UAV_02;
	FRDGTextureRef PixelAttributeA_03;
	FRDGTextureUAVRef PixelAttributeA_UAV_03;

	FRDGTextureRef PixelAttributeB_00;
	FRDGTextureUAVRef PixelAttributeB_UAV_00;
	FRDGTextureRef PixelAttributeB_01;
	FRDGTextureUAVRef PixelAttributeB_UAV_01;
	FRDGTextureRef PixelAttributeB_02;
	FRDGTextureUAVRef PixelAttributeB_UAV_02;
	FRDGTextureRef PixelAttributeB_03;
	FRDGTextureUAVRef PixelAttributeB_UAV_03;
};
