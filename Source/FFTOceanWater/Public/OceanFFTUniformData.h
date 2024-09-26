#pragma once
#include "ShaderParameterMacros.h"
class UOceanDataComponent;

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FOceanBasicUniformBufferData, )
SHADER_PARAMETER(FVector4f, Amplitude)
SHADER_PARAMETER(FVector4f, Chop)
SHADER_PARAMETER(FVector4f, PatchLength)
SHADER_PARAMETER(FVector4f, LongWaveCutOff)
SHADER_PARAMETER(FVector4f, ShortWaveCutOff)
SHADER_PARAMETER(FVector4f, WindDirectionality)
SHADER_PARAMETER(FVector4f, WindTighten)
SHADER_PARAMETER(FVector4f, FoamInjection)
SHADER_PARAMETER(FVector4f, FoamThreshlod)
SHADER_PARAMETER(FVector4f, FoamFade)
SHADER_PARAMETER(FVector4f, FoamBlur)
SHADER_PARAMETER(FVector2f, WindDirection)
SHADER_PARAMETER(float, AnimationTime)
SHADER_PARAMETER(float,	DeltaTime)
SHADER_PARAMETER(float, BaseFrequency)
SHADER_PARAMETER(float, WindSpeed)
SHADER_PARAMETER(float, Gravity)
SHADER_PARAMETER(float, RepeatPeriod)
SHADER_PARAMETER(int32, NumCascades)
END_GLOBAL_SHADER_PARAMETER_STRUCT()


using FOceanBasicUniformBufferRef = TUniformBufferRef<FOceanBasicUniformBufferData>;


extern void SetupOceanUniformParameters(FRDGBuilder& GraphBuilder, const UOceanDataComponent& OceanDataComponent, FOceanBasicUniformBufferData& OutParameters);


extern TRDGUniformBufferRef<FOceanBasicUniformBufferData> CreateOceanUniformBuffer(FRDGBuilder& GraphBuilder, const UOceanDataComponent& OceanDataComponent);


