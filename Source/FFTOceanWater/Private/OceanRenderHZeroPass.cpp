#include "OceanRenderHZeroPass.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Runtime/RHI/Public/RHIResources.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RHICommandList.h"
#include "OceanFFTUniformData.h"


class FOceeanComputeShader_PhlipCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOceeanComputeShader_PhlipCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FOceeanComputeShader_PhlipCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PhlipSurface)
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
IMPLEMENT_SHADER_TYPE(,FOceeanComputeShader_PhlipCS, TEXT("/Plugin/FFTOceanWater/HZeroGenerator.usf"), TEXT("ComputeHZero"), SF_Compute);

OceanRenderHZeroPass::OceanRenderHZeroPass()
{
}


void OceanRenderHZeroPass::Draw(FRHICommandListImmediate& RHICommandList,const FOceanRenderHZeroPassData& SetupData,const UOceanDataComponent& OceanDataComponent)
{
	FRDGBuilder GraphBuilder(RHICommandList);
	FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
			FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY),
			SetupData.OutputUAVFormat,
			FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));

	SpectrumTexture = GraphBuilder.CreateTexture(Desc, TEXT("HZeroResource_OutputSurfaceTexture"));
	SpectrumTextureUAV = GraphBuilder.CreateUAV(SpectrumTexture);

	TShaderMapRef<FOceeanComputeShader_PhlipCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
	FOceeanComputeShader_PhlipCS::FParameters* OceanHZeroParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_PhlipCS::FParameters>();

	OceanHZeroParameters->PhlipSurface = SpectrumTextureUAV;
	OceanHZeroParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

	
	auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_PhlipCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_PhlipCS::ThreadY, 1);
	GraphBuilder.AddPass(
	RDG_EVENT_NAME("HZeroComputeShader"),
	OceanHZeroParameters,
	ERDGPassFlags::AsyncCompute,
	[&OceanHZeroParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
	{
		FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanHZeroParameters,GroupCount);
	});
	
	GraphBuilder.QueueTextureExtraction(SpectrumTexture, &OutputRT);
	GraphBuilder.Execute();
}


