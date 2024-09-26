// Fill out your copyright notice in the Description page of Project Settings.

#include "OceanDataComponent.h"
#include "OceanRender.h"
#include "OceanFFTCommonData.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/TextureRenderTarget2DArray.h"
#include "Engine/World.h" 


// Sets default values for this component's properties
UOceanDataComponent::UOceanDataComponent()
{
	
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bSimulate = true;
	OcenRenderer = MakeUnique<OceanRender>();
	
	Amplitude = FVector4f(840000.0f, 32000.f, 2000.0f, 120.0f);
	PatchLength = FVector4f(10.f,28.f,432.f,2000.f);
	Choppiness = FVector4f(1.5f,1.6f,1.6f,1.6f);
	ShortWaveCutOff = FVector4f(0.002f,0.008f,2.f,64.f);
	LongWaveCutOff = FVector4f(1.f,0.2f,0.1f,0.03f);
	WindDirectionality = FVector4f(0.5f,0.5f,0.5f,0.5f);
	WindTighten = FVector4f(2.0f,2.0f,2.0f,2.0f);
	FoamInjection = FVector4f(1.0f,1.0f,1.0f,1.0f);
	FoamThreshlod = FVector4f(-0.2f,-0.2f,-0.2f,-0.2f);
	FoamFade = FVector4f(0.1f,0.1f,0.1f,0.1f);
	FoamBlur = FVector4f(2.0f,2.0f,2.0f,2.0f);
	WindSpeed = 33.f;
	WindDirection = 90.f;
	Gravity = 9.8f;
	RepeatPeriod = 1000.f;
	NumCascades = 4;
}


// Called when the game starts
void UOceanDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UOceanDataComponent::PostLoad()
{
	Super::PostLoad();
	VertexRenderTargets[0] = Displacement->GameThread_GetRenderTargetResource();
	VertexRenderTargets[1] = Displacement_Previous->GameThread_GetRenderTargetResource();

	PixelRenderTargets_A[0] = PixelAttrbuteA_00->GameThread_GetRenderTargetResource();
	PixelRenderTargets_A[1] = PixelAttrbuteA_01->GameThread_GetRenderTargetResource();
	PixelRenderTargets_A[2] = PixelAttrbuteA_02->GameThread_GetRenderTargetResource();
	PixelRenderTargets_A[3] = PixelAttrbuteA_03->GameThread_GetRenderTargetResource();
	
	PixelRenderTargets_B[0] = PixelAttrbuteB_00->GameThread_GetRenderTargetResource();
	PixelRenderTargets_B[1] = PixelAttrbuteB_01->GameThread_GetRenderTargetResource();
	PixelRenderTargets_B[2] = PixelAttrbuteB_02->GameThread_GetRenderTargetResource();
	PixelRenderTargets_B[3] = PixelAttrbuteB_03->GameThread_GetRenderTargetResource();
}

void UOceanDataComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = PropertyChangedEvent.Property->GetFName();
	if(PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,PatchLength)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,WindSpeed)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,Gravity)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,WindDirection)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,WindTighten)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,WindDirectionality)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,ShortWaveCutOff)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(UOceanDataComponent,LongWaveCutOff)
		)
	{
		bHzeroInitSuccess = false;
	}
}

// Called every frame
void UOceanDataComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	RenderOcean(DeltaTime);
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UOceanDataComponent::RenderOcean(float DeltaTime)
{
	if (bSimulate == false)
    {
        return;
    }
	OceanRenderData.SetFeatureLevel(GetWorld()->GetFeatureLevel());
	
	float TimeTotal = FMath::Min(RepeatPeriod,DeltaTime)+TimeValue;
	TimeValue = TimeTotal>RepeatPeriod?TimeTotal-TimeValue:TimeTotal;
	DeltaTimeValue = DeltaTime;
	
	OcenRenderer->Dispatch(OceanRenderData,*this,
		DebugRenderTarget2D_00->GameThread_GetRenderTargetResource(),DebugRenderTarget2D_01->GameThread_GetRenderTargetResource());
	
}

bool UOceanDataComponent::GetHzeroInitState() const
{
	return bHzeroInitSuccess;
}

void UOceanDataComponent::SetHzeroInitState(bool State)
{
	bHzeroInitSuccess = State;
}





