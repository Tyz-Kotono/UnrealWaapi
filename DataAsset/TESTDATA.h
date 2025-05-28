// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TESTDATA.generated.h"

/**
 * 
 */
UCLASS()
class UNREALWAPPI_API UTESTDATA : public UDataAsset
{
	GENERATED_BODY()
	UFUNCTION(CallInEditor, Category = "WwiseData", meta = (FullyExpand = true))
	void UpdateWwiseData();
};
