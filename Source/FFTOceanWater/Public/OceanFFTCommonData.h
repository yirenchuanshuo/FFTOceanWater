#pragma once

struct FOceanRenderHZeroPassData
{
	int32 OutputSizeX = 256;
	int32 OutputSizeY = 1024;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel;
};

struct FOceanRenderFrequencySpectrumPassData
{
	int32 OutputSizeX = 256;
	int32 OutputSizeY = 1024;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel;
	FRHITexture* HZeroTexture;
};

struct FOceanRenderIFFTPassData
{
	int32 OutputSizeX = 256;
	int32 OutputSizeY = 1024;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel;

	FRHITexture* FrequencySpectrumXYTexture;
	FRHITexture* FrequencySpectrumZTexture;
};

struct FOceanRenderExportDataPassData
{
	int32 OutputSizeX = 256;
	int32 OutputSizeY = 1024;
	EPixelFormat OutputUAVFormat = PF_FloatRGBA;
	ERHIFeatureLevel::Type FeatureLevel;
	
	FRHITexture* DisplacementTexture;
	FRHITexture* DisplacementTexture_Previous = GBlackTexture->GetTextureRHI();
	FRHITexture* Foam_Previous = GBlackTexture->GetTextureRHI();
};

struct FOceanRenderData
{
	FOceanRenderHZeroPassData OceanHZeroPassData;
	FOceanRenderFrequencySpectrumPassData OceanFrequencySpectrumPassData;
	FOceanRenderIFFTPassData OceanIFFTPassData;
	FOceanRenderExportDataPassData OceanExportDataPassData;

	void SetFeatureLevel(ERHIFeatureLevel::Type InFeatureLevel)
	{
		OceanHZeroPassData.FeatureLevel = InFeatureLevel;
		OceanFrequencySpectrumPassData.FeatureLevel = InFeatureLevel;
		OceanIFFTPassData.FeatureLevel = InFeatureLevel;
		OceanExportDataPassData.FeatureLevel = InFeatureLevel;
	}
};