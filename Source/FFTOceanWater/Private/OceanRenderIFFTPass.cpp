
#include "OceanRenderIFFTPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "OceanFFTUniformData.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

class FOceeanComputeShader_IFFTSpectrumCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOceeanComputeShader_IFFTSpectrumCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FOceeanComputeShader_IFFTSpectrumCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, DisplacementUAV)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, IFFTXYTextureUAV)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, IFFTZTextureUAV)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, IFFTXYTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, IFFTZTexture)
		SHADER_PARAMETER(uint32, Dir)
		SHADER_PARAMETER_RDG_UNIFORM_BUFFER(FOceanBasicUniformBufferData, OceanBasicUniformBufferData)
	END_SHADER_PARAMETER_STRUCT()
	
public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("THREADGROUP_SIZE_Z"), ThreadZ);
	}


	static constexpr uint32 ThreadX = 256;
	static constexpr uint32 ThreadY = 1;
	static constexpr uint32 ThreadZ = 1;
};
IMPLEMENT_SHADER_TYPE(,FOceeanComputeShader_IFFTSpectrumCS, TEXT("/Plugin/FFTOceanWater/IFFTSpectrum.usf"), TEXT("ComputeIFFTSpectrum"), SF_Compute);

OceanRenderIFFTPass::OceanRenderIFFTPass()
{
}

void OceanRenderIFFTPass::Draw(FRHICommandListImmediate& RHICommandList, const FOceanRenderIFFTPassData& SetupData,
                               const UOceanDataComponent& OceanDataComponent)
{
	
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
			FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY),
			SetupData.OutputUAVFormat,
			FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));

	TShaderMapRef<FOceeanComputeShader_IFFTSpectrumCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_IFFTSpectrumCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_IFFTSpectrumCS::ThreadY, 1);
	
	TRefCountPtr<IPooledRenderTarget> OutputIFFTXYTexture;
	TRefCountPtr<IPooledRenderTarget> OutputIFFTZTexture;
	
	{
		FRDGBuilder GraphBuilder(RHICommandList);
		FRDGTextureRef IFFTXYTexture_Previous = RegisterExternalTexture(GraphBuilder,SetupData.FrequencySpectrumXYTexture,TEXT("OceanRenderIFFTSpectrumPass_FFTTextureXY_Previous"));
		FRDGTextureRef IFFTZTexture_Previous = RegisterExternalTexture(GraphBuilder,SetupData.FrequencySpectrumZTexture,TEXT("OceanRenderIFFTSpectrumPass_FFTTextureZ_Previous"));

		IFFTXYTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_FFTTextureXY"));
		IFFTZTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_FFTTextureZ"));
		
		IFFTXYTextureUAV = GraphBuilder.CreateUAV(IFFTXYTexture);
		IFFTZTextureUAV = GraphBuilder.CreateUAV(IFFTZTexture);

		DisplacementTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_DisplacementTexture"));
		DisplacementTextureUAV = GraphBuilder.CreateUAV(DisplacementTexture);

		FOceeanComputeShader_IFFTSpectrumCS::FParameters* OceanIFFTSpectrumParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_IFFTSpectrumCS::FParameters>();
		OceanIFFTSpectrumParameters->DisplacementUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->IFFTXYTexture = IFFTXYTexture_Previous;
		OceanIFFTSpectrumParameters->IFFTZTexture = IFFTZTexture_Previous;
		OceanIFFTSpectrumParameters->IFFTXYTextureUAV = IFFTXYTextureUAV;
		OceanIFFTSpectrumParameters->IFFTZTextureUAV = IFFTZTextureUAV;
		OceanIFFTSpectrumParameters->Dir = 0;
		OceanIFFTSpectrumParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

		ClearUnusedGraphResources(OceanComputeShader, OceanIFFTSpectrumParameters);
		
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("IFFTSpectrumComputeShaderRow"),
		OceanIFFTSpectrumParameters,
		ERDGPassFlags::AsyncCompute,
		[&OceanIFFTSpectrumParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanIFFTSpectrumParameters,GroupCount);
		});
		
		GraphBuilder.QueueTextureExtraction(IFFTXYTexture, &OutputIFFTXYTexture);
		GraphBuilder.QueueTextureExtraction(IFFTZTexture, &OutputIFFTZTexture);
		GraphBuilder.Execute();
	}
	
	
	{
		FRDGBuilder GraphBuilder(RHICommandList);
		DisplacementTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_DisplacementTexture"));
		DisplacementTextureUAV = GraphBuilder.CreateUAV(DisplacementTexture);
		
		
		FOceeanComputeShader_IFFTSpectrumCS::FParameters* OceanIFFTSpectrumParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_IFFTSpectrumCS::FParameters>();
		OceanIFFTSpectrumParameters->DisplacementUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->IFFTXYTexture =   GraphBuilder.RegisterExternalTexture(OutputIFFTXYTexture);
		OceanIFFTSpectrumParameters->IFFTZTexture = GraphBuilder.RegisterExternalTexture(OutputIFFTZTexture);
		OceanIFFTSpectrumParameters->IFFTXYTextureUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->IFFTZTextureUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->Dir = 1;
		OceanIFFTSpectrumParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

		ClearUnusedGraphResources(OceanComputeShader, OceanIFFTSpectrumParameters);
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("IFFTSpectrumComputeShaderCol"),
		OceanIFFTSpectrumParameters,
		ERDGPassFlags::AsyncCompute,
		[&OceanIFFTSpectrumParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanIFFTSpectrumParameters,GroupCount);
		});
		GraphBuilder.QueueTextureExtraction(DisplacementTexture, &OutputRTDisplacement);
		GraphBuilder.Execute();
	}
	
	
	
	
}
