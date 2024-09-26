// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "OceanFFTCommonData.h"
#include "OceanRender.h"
#include "Components/ActorComponent.h"
#include "OceanDataComponent.generated.h"


class UTextureRenderTarget2DArray;

UCLASS(hidecategories = (Object, LOD, Physics, Collision), editinlinenew,ClassGroup=(Rendering), meta=(BlueprintSpawnableComponent))
class FFTOCEANWATER_API UOceanDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UOceanDataComponent();

protected:
	virtual void BeginPlay() override;
	
	virtual void PostLoad() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
public:
	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
	FVector4f Amplitude;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
	FVector4f WindDirectionality;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
	FVector4f Choppiness;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
	FVector4f PatchLength;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
    FVector4f ShortWaveCutOff;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
    FVector4f LongWaveCutOff;

	UPROPERTY(EditAnywhere, Category = "OceanDataCascade")
	FVector4f WindTighten;

	UPROPERTY(EditAnywhere, Category = "OceanDataFoam")
	FVector4f FoamInjection;

	UPROPERTY(EditAnywhere, Category = "OceanDataFoam")
	FVector4f FoamThreshlod;

	UPROPERTY(EditAnywhere, Category = "OceanDataFoam")
	FVector4f FoamFade;

	UPROPERTY(EditAnywhere, Category = "OceanDataFoam")
	FVector4f FoamBlur;

	UPROPERTY(EditAnywhere, Category = "OceanDataWind")
	float WindSpeed;

	UPROPERTY(EditAnywhere, Category = "OceanDataWind")
	float WindDirection;

	UPROPERTY(EditAnywhere, Category = "OceanDataMisc")
	float Gravity;

	UPROPERTY(EditAnywhere, Category = "OceanDataMisc")
	float RepeatPeriod;

	UPROPERTY(EditAnywhere, Category = "OceanComponent")
	bool bSimulate;

	
public:
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2DArray> Displacement;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2DArray> Displacement_Previous;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteA_00;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteA_01;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteA_02;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteA_03;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteB_00;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteB_01;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteB_02;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> PixelAttrbuteB_03;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> DebugRenderTarget2D_00;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "OceanData")
	TObjectPtr<UTextureRenderTarget2D> DebugRenderTarget2D_01;

	TStaticArray<FTextureRenderTargetResource*,2> VertexRenderTargets;
	TStaticArray<FTextureRenderTargetResource*,4> PixelRenderTargets_A;
	TStaticArray<FTextureRenderTargetResource*,4> PixelRenderTargets_B;
	
	TUniquePtr<OceanRender> OcenRenderer;
	float TimeValue;
	float DeltaTimeValue;
	uint32 NumCascades;
	
	FOceanRenderData OceanRenderData;

	
	UFUNCTION(BlueprintCallable, Category = "OceanRender")
	void RenderOcean(float DeltaTime);

	bool GetHzeroInitState()const;
	void SetHzeroInitState(bool State);
	
private:
	bool bHzeroInitSuccess;
};


