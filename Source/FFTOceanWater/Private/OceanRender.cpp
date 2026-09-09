#include "OceanRender.h"
#include "OceanDataComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"


OceanRender::OceanRender()
{
	OceanHZeroPass = MakeUnique<OceanRenderHZeroPass>();
	FrequencyPass = MakeUnique<OceanRenderFrequencySpectrumPass>();
	IFFTPass = MakeUnique<OceanRenderIFFTPass>();
	ExportDataPass = MakeUnique<OceanRenderExportDataPass>();
}

void OceanRender::Draw(FRHICommandListImmediate& RHICommandList,FOceanRenderData& SetupData, UOceanDataComponent& OceanDataComponent,
	FTextureRenderTargetResource* DebugRenderTargetRHITexture ,FTextureRenderTargetResource* DebugRenderTargetRHITexture2)
{
	// A single FRDGBuilder for the whole ocean-simulation frame.
	// Every pass is enqueued via its AddPass() method and executed once at the end.
	FRDGBuilder GraphBuilder(RHICommandList);

	// -------- HZero --------
	// HZero is generated once (initialization) and then reused across frames via OutputRT.
	FRDGTextureRef HZeroTexture = nullptr;
	if (OceanDataComponent.GetHzeroInitState() == false)
	{
		OceanHZeroPass->AddPass(GraphBuilder, SetupData.OceanHZeroPassData, OceanDataComponent);
		HZeroTexture = OceanHZeroPass->SpectrumTexture;
		OceanDataComponent.SetHzeroInitState(true);
	}
	else
	{
		check(OceanHZeroPass->OutputRT.IsValid());
		HZeroTexture = GraphBuilder.RegisterExternalTexture(OceanHZeroPass->OutputRT, TEXT("HZeroTexture_Cached"));
	}

	// -------- Frequency spectrum --------
	FrequencyPass->AddPass(GraphBuilder, SetupData.OceanFrequencySpectrumPassData, OceanDataComponent, HZeroTexture);

	// -------- IFFT (Row + Column) --------
	IFFTPass->AddPass(GraphBuilder, SetupData.OceanIFFTPassData, OceanDataComponent,
		FrequencyPass->FFTXYTexture, FrequencyPass->FFTZTexture);

	// -------- Export data (vertex + pixel) --------
	ExportDataPass->AddPass(GraphBuilder, SetupData.OceanExportDataPassData, OceanDataComponent,
		IFFTPass->DisplacementTexture);

	// -------- Debug outputs --------
	if (DebugRenderTargetRHITexture && DebugRenderTargetRHITexture2)
	{
		FRDGTextureRef DebugRT0 = RegisterExternalTexture(GraphBuilder,
			DebugRenderTargetRHITexture->GetRenderTargetTexture(),
			TEXT("OceanDebugRT_00"));
		FRDGTextureRef DebugRT1 = RegisterExternalTexture(GraphBuilder,
			DebugRenderTargetRHITexture2->GetRenderTargetTexture(),
			TEXT("OceanDebugRT_01"));
		AddCopyTexturePass(GraphBuilder, FrequencyPass->FFTXYTexture, DebugRT0, FRHICopyTextureInfo());
		AddCopyTexturePass(GraphBuilder, FrequencyPass->FFTZTexture,  DebugRT1, FRHICopyTextureInfo());
	}

	// Execute all recorded passes at once.
	GraphBuilder.Execute();
}

void OceanRender::Dispatch(FOceanRenderData& SetupData, UOceanDataComponent& OceanDataComponent,
	FTextureRenderTargetResource* DebugRenderTargetRHITexture,FTextureRenderTargetResource* DebugRenderTargetRHITexture2)
{
	if(IsInRenderingThread())
	{
		FRHICommandListImmediate& RHICmdList = GetImmediateCommandList_ForRenderCommand();
		if(IFFTPass->OutputRTDisplacement.IsValid())
		{
			SetupData.OceanExportDataPassData.DisplacementTexture_Previous = IFFTPass->OutputRTDisplacement->GetRHI();
		}
		if(ExportDataPass->PixelData_B_OutPut.IsValid())
		{
			SetupData.OceanExportDataPassData.Foam_Previous = ExportDataPass->PixelData_B_OutPut->GetRHI();
		}
		Draw(RHICmdList,SetupData,OceanDataComponent,DebugRenderTargetRHITexture,DebugRenderTargetRHITexture2);
	}
	else
	{
		ENQUEUE_RENDER_COMMAND(OceanRenderHZeroTextureCommand)(
	[&SetupData,&OceanDataComponent,DebugRenderTargetRHITexture,DebugRenderTargetRHITexture2,this](FRHICommandListImmediate& RHICmdList)
		{
			if(IFFTPass->OutputRTDisplacement.IsValid())
			{
				SetupData.OceanExportDataPassData.DisplacementTexture_Previous = IFFTPass->OutputRTDisplacement->GetRHI();
			}
			if(ExportDataPass->PixelData_B_OutPut.IsValid())
			{
				SetupData.OceanExportDataPassData.Foam_Previous = ExportDataPass->PixelData_B_OutPut->GetRHI();
			}
			Draw(RHICmdList,SetupData,OceanDataComponent,DebugRenderTargetRHITexture,DebugRenderTargetRHITexture2);
		});
	}
}


