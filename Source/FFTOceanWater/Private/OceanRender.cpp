#include "OceanRender.h"
#include "OceanDataComponent.h"
#include "Engine/TextureRenderTarget2D.h"


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
	if(OceanDataComponent.GetHzeroInitState() == false)
	{
		OceanHZeroPass->Draw(RHICommandList,SetupData.OceanHZeroPassData,OceanDataComponent);
		OceanDataComponent.SetHzeroInitState(true);
	}
	
	SetupData.OceanFrequencySpectrumPassData.HZeroTexture = OceanHZeroPass->OutputRT->GetRHI();
	FrequencyPass->Draw(RHICommandList,SetupData.OceanFrequencySpectrumPassData,OceanDataComponent);

	SetupData.OceanIFFTPassData.FrequencySpectrumXYTexture = FrequencyPass->OutputRTXY->GetRHI();
	SetupData.OceanIFFTPassData.FrequencySpectrumZTexture = FrequencyPass->OutputRTZ->GetRHI();
	
	IFFTPass->Draw(RHICommandList,SetupData.OceanIFFTPassData,OceanDataComponent);

	SetupData.OceanExportDataPassData.DisplacementTexture = IFFTPass->OutputRTDisplacement->GetRHI();
	

	ExportDataPass->Draw(RHICommandList,SetupData.OceanExportDataPassData,OceanDataComponent);
	
	RHICommandList.CopyTexture(FrequencyPass->OutputRTXY->GetRHI(), OceanDataComponent.DebugRenderTarget2D_00->GetRenderTargetResource()->GetTexture2DRHI(), FRHICopyTextureInfo());
	RHICommandList.CopyTexture(FrequencyPass->OutputRTZ->GetRHI(), OceanDataComponent.DebugRenderTarget2D_01->GetRenderTargetResource()->GetTexture2DRHI(), FRHICopyTextureInfo());
	
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


