#include "OceanRenderExportDataPass.h"
#include "OceanFFTUniformData.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "OceanDataComponent.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "ShaderParameterStruct.h"

class FOceeanComputeShader_VertexDataExportCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOceeanComputeShader_VertexDataExportCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FOceeanComputeShader_VertexDataExportCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray, DisplacementTextureOutput)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2DArray, DisplacementTextureOutput_Previous)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelData_A)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelData_B)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, DisplacementTexture)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, DisplacementTexture_Previous)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, Foam_Previous)
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
		OutEnvironment.SetDefine(TEXT("VERTEXDATA_SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("VERTEXDATA_SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("VERTEXDATA_SIZE_Z"), ThreadZ);
	}


	static constexpr uint32 ThreadX = 16;
	static constexpr uint32 ThreadY = 16;
	static constexpr uint32 ThreadZ = 4;
};
IMPLEMENT_SHADER_TYPE(,FOceeanComputeShader_VertexDataExportCS, TEXT("/Plugin/FFTOceanWater/OceanDataExport.usf"), TEXT("OceanVertexDataExport"), SF_Compute);

class FOceeanComputeShader_PixelDataExportCS : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FOceeanComputeShader_PixelDataExportCS, Global);
	SHADER_USE_PARAMETER_STRUCT(FOceeanComputeShader_PixelDataExportCS, FGlobalShader);
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeA_00)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeA_01)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeA_02)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeA_03)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeB_00)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeB_01)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeB_02)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, PixelAttributeB_03)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, AttributesA)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, AttributesB)
	END_SHADER_PARAMETER_STRUCT()

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("PIXLEDATA_SIZE_X"), ThreadX);
		OutEnvironment.SetDefine(TEXT("PIXLEDATA_SIZE_Y"), ThreadY);
		OutEnvironment.SetDefine(TEXT("PIXLEDATA_SIZE_Z"), ThreadZ);
	}

	static constexpr uint32 ThreadX = 16;
	static constexpr uint32 ThreadY = 16;
	static constexpr uint32 ThreadZ = 1;
};
IMPLEMENT_SHADER_TYPE(,FOceeanComputeShader_PixelDataExportCS, TEXT("/Plugin/FFTOceanWater/OceanDataExport.usf"), TEXT("OceanPixelDataExport"), SF_Compute);


void OceanRenderExportDataPass::Draw(FRHICommandListImmediate& RHICommandList , const FOceanRenderExportDataPassData& SetupData , const UOceanDataComponent& OceanDataComponent)
{
	{
		FRDGBuilder GraphBuilder(RHICommandList);
		FRDGTextureDesc DescFoam(FRDGTextureDesc::Create2D(
				FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY),
				SetupData.OutputUAVFormat,
				FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));

		FRDGTextureDesc DescSpecArray(FRDGTextureDesc::Create2DArray(
				FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY/4),
				SetupData.OutputUAVFormat,
				FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV,4));

		DisplacementTextureOutput = GraphBuilder.CreateTexture(DescSpecArray, TEXT("OceanRenderExportDataPass_DisplacementTextureOutput"));
		DisplacementTextureOutputUAV = GraphBuilder.CreateUAV(DisplacementTextureOutput);

		DisplacementTextureOutput_Previous = GraphBuilder.CreateTexture(DescSpecArray, TEXT("OceanRenderExportDataPass_DisplacementTextureOutput_Previous"));
		DisplacementTextureOutput_PreviousUAV = GraphBuilder.CreateUAV(DisplacementTextureOutput_Previous);

		PixelData_A = GraphBuilder.CreateTexture(DescFoam, TEXT("OceanRenderExportDataPass_PixelData_A"));
		PixelDataUAV_A = GraphBuilder.CreateUAV(PixelData_A);

		PixelData_B = GraphBuilder.CreateTexture(DescFoam, TEXT("OceanRenderExportDataPass_PixelData_B"));
		PixelDataUAV_B = GraphBuilder.CreateUAV(PixelData_B);
		

		TShaderMapRef<FOceeanComputeShader_VertexDataExportCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		FOceeanComputeShader_VertexDataExportCS::FParameters* OceanVertexDataExportParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_VertexDataExportCS::FParameters>();

		OceanVertexDataExportParameters->DisplacementTextureOutput = DisplacementTextureOutputUAV;
		OceanVertexDataExportParameters->DisplacementTextureOutput_Previous = DisplacementTextureOutput_PreviousUAV;
		OceanVertexDataExportParameters->PixelData_A = PixelDataUAV_A;
		OceanVertexDataExportParameters->PixelData_B = PixelDataUAV_B;
		OceanVertexDataExportParameters->DisplacementTexture = RegisterExternalTexture(GraphBuilder,SetupData.DisplacementTexture,TEXT("DisplacementTexture"));
		OceanVertexDataExportParameters->DisplacementTexture_Previous = RegisterExternalTexture(GraphBuilder,SetupData.DisplacementTexture_Previous,TEXT("DisplacementTexture_Previous"));
		OceanVertexDataExportParameters->Foam_Previous = RegisterExternalTexture(GraphBuilder,SetupData.Foam_Previous,TEXT("FoamTexture_Previous"));
		OceanVertexDataExportParameters->OceanBasicUniformBufferData = CreateOceanUniformBuffer(GraphBuilder,OceanDataComponent);

		
		auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_VertexDataExportCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_VertexDataExportCS::ThreadY, 1);
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("VertexDataExportComputeShader"),
		OceanVertexDataExportParameters,
		ERDGPassFlags::AsyncCompute,
		[&OceanVertexDataExportParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanVertexDataExportParameters,GroupCount);
		});

		
		FRHICopyTextureInfo CopyInfo;
		CopyInfo.NumSlices = 4;
		FRDGTextureRef VertexAttributesA = RegisterExternalTexture(GraphBuilder, OceanDataComponent.VertexRenderTargets[0]->GetRenderTargetTexture(), TEXT("DisplacementTextureOutput_RT"));
		FRDGTextureRef VertexAttributesB = RegisterExternalTexture(GraphBuilder, OceanDataComponent.VertexRenderTargets[1]->GetRenderTargetTexture(), TEXT("DisplacementTextureOutput_Previous_RT"));
		AddCopyTexturePass(GraphBuilder, DisplacementTextureOutput, VertexAttributesA,CopyInfo);
		AddCopyTexturePass(GraphBuilder, DisplacementTextureOutput_Previous, VertexAttributesB,CopyInfo);

		GraphBuilder.QueueTextureExtraction(PixelData_A, &PixelData_A_OutPut);	
		GraphBuilder.QueueTextureExtraction(PixelData_B, &PixelData_B_OutPut);
		
		GraphBuilder.Execute();
	}
	
	{
		FRDGBuilder GraphBuilder(RHICommandList);
		FRDGTextureDesc Desc(FRDGTextureDesc::Create2D(
				FIntPoint(SetupData.OutputSizeX, SetupData.OutputSizeY/4),
				SetupData.OutputUAVFormat,
				FClearValueBinding::White, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
		PixelAttributeA_00 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeA_00"));
		PixelAttributeA_UAV_00 = GraphBuilder.CreateUAV(PixelAttributeA_00);

		PixelAttributeA_01 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeA_01"));
		PixelAttributeA_UAV_01 = GraphBuilder.CreateUAV(PixelAttributeA_01);

		PixelAttributeA_02 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeA_02"));
		PixelAttributeA_UAV_02 = GraphBuilder.CreateUAV(PixelAttributeA_02);

		PixelAttributeA_03 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeA_03"));
		PixelAttributeA_UAV_03 = GraphBuilder.CreateUAV(PixelAttributeA_03);

		PixelAttributeB_00 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeB_00"));
		PixelAttributeB_UAV_00 = GraphBuilder.CreateUAV(PixelAttributeB_00);

		PixelAttributeB_01 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeB_01"));
		PixelAttributeB_UAV_01 = GraphBuilder.CreateUAV(PixelAttributeB_01);

		PixelAttributeB_02 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeB_02"));
		PixelAttributeB_UAV_02 = GraphBuilder.CreateUAV(PixelAttributeB_02);

		PixelAttributeB_03 = GraphBuilder.CreateTexture(Desc, TEXT("OceanRenderExportDataPass_PixelAttributeB_03"));
		PixelAttributeB_UAV_03 = GraphBuilder.CreateUAV(PixelAttributeB_03);

		TShaderMapRef<FOceeanComputeShader_PixelDataExportCS> OceanComputeShader(GetGlobalShaderMap(SetupData.FeatureLevel));
		FOceeanComputeShader_PixelDataExportCS::FParameters* OceanPixelDataExportParameters = GraphBuilder.AllocParameters<FOceeanComputeShader_PixelDataExportCS::FParameters>();

		OceanPixelDataExportParameters->PixelAttributeA_00 = PixelAttributeA_UAV_00;
		OceanPixelDataExportParameters->PixelAttributeA_01 = PixelAttributeA_UAV_01;
		OceanPixelDataExportParameters->PixelAttributeA_02 = PixelAttributeA_UAV_02;
		OceanPixelDataExportParameters->PixelAttributeA_03 = PixelAttributeA_UAV_03;
		OceanPixelDataExportParameters->PixelAttributeB_00 = PixelAttributeB_UAV_00;
		OceanPixelDataExportParameters->PixelAttributeB_01 = PixelAttributeB_UAV_01;
		OceanPixelDataExportParameters->PixelAttributeB_02 = PixelAttributeB_UAV_02;
		OceanPixelDataExportParameters->PixelAttributeB_03 = PixelAttributeB_UAV_03;
		OceanPixelDataExportParameters->AttributesA = RegisterExternalTexture(GraphBuilder,PixelData_A_OutPut->GetRHI(),TEXT("AttributesA"));
		OceanPixelDataExportParameters->AttributesB = RegisterExternalTexture(GraphBuilder,PixelData_B_OutPut->GetRHI(),TEXT("AttributesB"));
		
		auto GroupCount = FIntVector(SetupData.OutputSizeX/FOceeanComputeShader_PixelDataExportCS::ThreadX, SetupData.OutputSizeY/4/FOceeanComputeShader_PixelDataExportCS::ThreadY, 1);
		GraphBuilder.AddPass(
		RDG_EVENT_NAME("PixelDataExportComputeShader"),
		OceanPixelDataExportParameters,
		ERDGPassFlags::AsyncCompute,
		[&OceanPixelDataExportParameters, OceanComputeShader,GroupCount](FRHIComputeCommandList& RHICmdList)
		{
			FComputeShaderUtils::Dispatch(RHICmdList, OceanComputeShader, *OceanPixelDataExportParameters,GroupCount);
		});

		FRHICopyTextureInfo CopyInfo;
		FRDGTextureRef PixelAttributeA_OutPut_00 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_A[0]->GetRenderTargetTexture(), TEXT("PixelAttributeA_OutPut_00"));
		FRDGTextureRef PixelAttributeA_OutPut_01 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_A[1]->GetRenderTargetTexture(), TEXT("PixelAttributeA_OutPut_01"));
		FRDGTextureRef PixelAttributeA_OutPut_02 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_A[2]->GetRenderTargetTexture(), TEXT("PixelAttributeA_OutPut_02"));
		FRDGTextureRef PixelAttributeA_OutPut_03 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_A[3]->GetRenderTargetTexture(), TEXT("PixelAttributeA_OutPut_03"));
		FRDGTextureRef PixelAttributeB_OutPut_00 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_B[0]->GetRenderTargetTexture(), TEXT("PixelAttributeB_OutPut_00"));
		FRDGTextureRef PixelAttributeB_OutPut_01 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_B[1]->GetRenderTargetTexture(), TEXT("PixelAttributeB_OutPut_01"));
		FRDGTextureRef PixelAttributeB_OutPut_02 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_B[2]->GetRenderTargetTexture(), TEXT("PixelAttributeB_OutPut_02"));
		FRDGTextureRef PixelAttributeB_OutPut_03 = RegisterExternalTexture(GraphBuilder, OceanDataComponent.PixelRenderTargets_B[3]->GetRenderTargetTexture(), TEXT("PixelAttributeB_OutPut_03"));
		AddCopyTexturePass(GraphBuilder, PixelAttributeA_00, PixelAttributeA_OutPut_00,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeA_01, PixelAttributeA_OutPut_01,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeA_02, PixelAttributeA_OutPut_02,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeA_03, PixelAttributeA_OutPut_03,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeB_00, PixelAttributeB_OutPut_00,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeB_01, PixelAttributeB_OutPut_01,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeB_02, PixelAttributeB_OutPut_02,CopyInfo);
		AddCopyTexturePass(GraphBuilder, PixelAttributeB_03, PixelAttributeB_OutPut_03,CopyInfo);
		GraphBuilder.Execute();
	}
}
