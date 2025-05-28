// Fill out your copyright notice in the Description page of Project Settings.


#include "TESTDATA.h"
#include "AkAudio/Public/AkWaapiClient.h"


//for Ak Name Space
#include <AK/WwiseAuthoringAPI/waapi.h>

#include "AkWaapiUtils.h"
#include "UObject/SavePackage.h"

void UTESTDATA::UpdateWwiseData()
{
	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> Result;

	bool bSuccess = FAkWaapiClient::Get()->Call(ak::wwise::core::getInfo, Args, Options, Result);

	if (bSuccess && Result.IsValid())
	{
		// 获取 displayName 字段
		TSharedPtr<FJsonObject> VersionObject = Result->GetObjectField("version");
		if (VersionObject.IsValid())
		{
			FString DisplayName = VersionObject->GetStringField("displayName");
			UE_LOG(LogTemp, Display, TEXT("Connected to Wwise: %s"), *DisplayName);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Version object not found in result."));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to call ak.wwise.core.getInfo. Is WAAPI running and enabled in Wwise?"));
	}
}
