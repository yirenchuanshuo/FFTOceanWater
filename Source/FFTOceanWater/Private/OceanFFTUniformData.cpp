#include "OceanFFTUniformData.h"
#include "OceanDataComponent.h"
#include "RenderGraphBuilder.h"


IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FOceanBasicUniformBufferData, "OceanBasicUniformBuffer");
void SetupOceanUniformParameters(const UOceanDataComponent& OceanDataComponent,FOceanBasicUniformBufferData& OutParameters)
{
	OutParameters.Amplitude = OceanDataComponent.Amplitude;
	OutParameters.Chop = OceanDataComponent.Choppiness;
	OutParameters.PatchLength = OceanDataComponent.PatchLength;
	OutParameters.LongWaveCutOff = OceanDataComponent.LongWaveCutOff;
	OutParameters.ShortWaveCutOff = OceanDataComponent.ShortWaveCutOff;
	OutParameters.WindDirectionality = FVector4f::One()-OceanDataComponent.WindDirectionality;
	OutParameters.WindTighten = OceanDataComponent.WindTighten;

	OutParameters.FoamInjection = OceanDataComponent.FoamInjection;
	OutParameters.FoamThreshlod = OceanDataComponent.FoamThreshlod;
	OutParameters.FoamFade = OceanDataComponent.FoamFade;
	OutParameters.FoamBlur = OceanDataComponent.FoamBlur;
	
	OutParameters.WindDirection = FVector2f(FMath::Sin(OceanDataComponent.WindDirection),FMath::Cos(OceanDataComponent.WindDirection));
	OutParameters.WindSpeed = OceanDataComponent.WindSpeed;
	OutParameters.Gravity = OceanDataComponent.Gravity;
	OutParameters.RepeatPeriod = OceanDataComponent.RepeatPeriod;
	OutParameters.BaseFrequency = UE_TWO_PI / OceanDataComponent.RepeatPeriod;
	OutParameters.AnimationTime = OceanDataComponent.TimeValue;
	OutParameters.DeltaTime = OceanDataComponent.DeltaTimeValue;
	OutParameters.NumCascades = OceanDataComponent.NumCascades;
}

TRDGUniformBufferRef<FOceanBasicUniformBufferData> CreateOceanUniformBuffer(FRDGBuilder& GraphBuilder,const UOceanDataComponent& OceanDataComponent)
{
	auto* OceanStruct = GraphBuilder.AllocParameters<FOceanBasicUniformBufferData>();
	SetupOceanUniformParameters(OceanDataComponent, *OceanStruct);
	return GraphBuilder.CreateUniformBuffer(OceanStruct);
}
