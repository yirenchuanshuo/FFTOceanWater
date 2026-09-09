
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

void OceanRenderIFFTPass::AddPass(FRDGBuilder& GraphBuilder,
                                  const FOceanRenderIFFTPassData& SetupData,
                                  const UOceanDataComponent& OceanDataComponent,
                                  FRDGTextureRef FrequencySpectrumXYInput,
                                  FRDGTextureRef FrequencySpectrumZInput)
{
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
			FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY),
			SetupData.OutputUAVFormat,
			FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));

	TShaderMapRef<FOceeanComputeShader_IFFTSpectrumCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_IFFTSpectrumCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_IFFTSpectrumCS::ThreadY, 1);

	// ---------------- Row pass (Dir = 0) ----------------
	IFFTXYTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_FFTTextureXY"));
	IFFTZTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_FFTTextureZ"));

	IFFTXYTextureUAV = GraphBuilder.CreateUAV(IFFTXYTexture);
	IFFTZTextureUAV = GraphBuilder.CreateUAV(IFFTZTexture);

	// Temporary displacement UAV for the row pass (unused output but the shader binds it).
	FRDGTextureRef DisplacementTextureRow = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_DisplacementTextureRow"));
	FRDGTextureUAVRef DisplacementTextureRowUAV = GraphBuilder.CreateUAV(DisplacementTextureRow);

	{
		FOceeanComputeShader_IFFTSpectrumCS::FParameters* OceanIFFTSpectrumParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_IFFTSpectrumCS::FParameters>();
		OceanIFFTSpectrumParameters->DisplacementUAV = DisplacementTextureRowUAV;
		OceanIFFTSpectrumParameters->IFFTXYTexture = FrequencySpectrumXYInput;
		OceanIFFTSpectrumParameters->IFFTZTexture = FrequencySpectrumZInput;
		OceanIFFTSpectrumParameters->IFFTXYTextureUAV = IFFTXYTextureUAV;
		OceanIFFTSpectrumParameters->IFFTZTextureUAV = IFFTZTextureUAV;
		OceanIFFTSpectrumParameters->Dir = 0;
		OceanIFFTSpectrumParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

		ClearUnusedGraphResources(OceanComputeShader, OceanIFFTSpectrumParameters);

		GraphBuilder.AddPass(
		RDG_EVENT_NAME("IFFTSpectrumComputeShaderRow"),
		OceanIFFTSpectrumParameters,
		ERDGPassFlags::AsyncCompute,
		[OceanIFFTSpectrumParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanIFFTSpectrumParameters,GroupCount);
		});
	}

	// ---------------- Column pass (Dir = 1) ----------------
	DisplacementTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderIFFTSpectrumPass_DisplacementTexture"));
	DisplacementTextureUAV = GraphBuilder.CreateUAV(DisplacementTexture);

	{
		FOceeanComputeShader_IFFTSpectrumCS::FParameters* OceanIFFTSpectrumParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_IFFTSpectrumCS::FParameters>();
		OceanIFFTSpectrumParameters->DisplacementUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->IFFTXYTexture = IFFTXYTexture;
		OceanIFFTSpectrumParameters->IFFTZTexture = IFFTZTexture;
		OceanIFFTSpectrumParameters->IFFTXYTextureUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->IFFTZTextureUAV = DisplacementTextureUAV;
		OceanIFFTSpectrumParameters->Dir = 1;
		OceanIFFTSpectrumParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

		ClearUnusedGraphResources(OceanComputeShader, OceanIFFTSpectrumParameters);
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("IFFTSpectrumComputeShaderCol"),
		OceanIFFTSpectrumParameters,
		ERDGPassFlags::AsyncCompute,
		[OceanIFFTSpectrumParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanIFFTSpectrumParameters,GroupCount);
		});
	}

	GraphBuilder.QueueTextureExtraction(DisplacementTexture, &OutputRTDisplacement);
}

void OceanRenderIFFTPass::Draw(FRHICommandListImmediate& RHICommandList, const FOceanRenderIFFTPassData& SetupData,
                               const UOceanDataComponent& OceanDataComponent)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureRef FreqXY = RegisterExternalTexture(GraphBuilder,SetupData.FrequencySpectrumXYTexture,TEXT("OceanRenderIFFTSpectrumPass_FFTTextureXY_Previous"));
	FRDGTextureRef FreqZ  = RegisterExternalTexture(GraphBuilder,SetupData.FrequencySpectrumZTexture,TEXT("OceanRenderIFFTSpectrumPass_FFTTextureZ_Previous"));
	AddPass(GraphBuilder, SetupData, OceanDataComponent, FreqXY, FreqZ);
	GraphBuilder.Execute();
}
