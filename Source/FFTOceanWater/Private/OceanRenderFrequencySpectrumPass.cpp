#include "OceanRenderFrequencySpectrumPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "ShaderParameterStruct.h"
#include "OceanFFTUniformData.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"


class FOceeanComputeShader_FrequencySpectrumCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOceeanComputeShader_FrequencySpectrumCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FOceeanComputeShader_FrequencySpectrumCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, FFTXYTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, FFTZTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, HZeroTexture)
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


	static constexpr uint32 ThreadX = 16;
	static constexpr uint32 ThreadY = 16;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FOceeanComputeShader_FrequencySpectrumCS, TEXT("/Plugin/FFTOceanWater/FrequencySpectrum.usf"), TEXT("ComputeFrequencySpectrum"), SF_Compute);

OceanRenderFrequencySpectrumPass::OceanRenderFrequencySpectrumPass()
{
}

void OceanRenderFrequencySpectrumPass::Draw(FRHICommandListImmediate& RHICommandList,
                                            const FOceanRenderFrequencySpectrumPassData& SetupData, const UOceanDataComponent& OceanDataComponent)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
			FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY),
			SetupData.OutputUAVFormat,
			FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));

	FFTXYTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderFrequencySpectrumPass_FFTTextureXY"));
	FFTZTexture = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderFrequencySpectrumPass_FFTTextureZ"));

	FFTXYTextureUAV = GraphBuilder.CreateUAV(FFTXYTexture);
	FFTZTextureUAV = GraphBuilder.CreateUAV(FFTZTexture);

	TShaderMapRef<FOceeanComputeShader_FrequencySpectrumCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FOceeanComputeShader_FrequencySpectrumCS::FParameters* OceanFrequencySpectrumParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_FrequencySpectrumCS::FParameters>();

	//Update uniform buffer
	HZeroTexture = RegisterExternalTexture(GraphBuilder,SetupData.HZeroTexture,TEXT("HZeroTexture"));
	OceanFrequencySpectrumParameters->FFTXYTexture = FFTXYTextureUAV;
	OceanFrequencySpectrumParameters->FFTZTexture = FFTZTextureUAV;
	OceanFrequencySpectrumParameters->HZeroTexture = HZeroTexture;
	OceanFrequencySpectrumParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

	auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_FrequencySpectrumCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_FrequencySpectrumCS::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("FrequencySpectrumComputeShader"),
	OceanFrequencySpectrumParameters,
	ERDGPassFlags::AsyncCompute,
	[&OceanFrequencySpectrumParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanFrequencySpectrumParameters,GroupCount);
	});
	
	GraphBuilder.QueueTextureExtraction(FFTXYTexture, &OutputRTXY);
	GraphBuilder.QueueTextureExtraction(FFTZTexture, &OutputRTZ);
	GraphBuilder.Execute();
}


