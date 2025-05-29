// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TESTDATA.generated.h"

/**
 * 
 */


#pragma region select
USTRUCT(BlueprintType)
struct FSelectionStruct
{
	GENERATED_BODY()

public:
	// 默认构造函数
	FSelectionStruct()
	{
	}

	// 自定义构造函数
	FSelectionStruct(FString name, FString id, FString path, FString type)
		: Name(name), ID(id), Path(path), Type(type)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Type;
};

#pragma endregion

#pragma region SoundBankGeneration


USTRUCT(BlueprintType)
struct FSoundBankGenerationSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise SoundBank")
	FString Platform = "Windows";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise SoundBank")
	TArray<FString> Languages = { "SFX" };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise SoundBank")
	bool bConvertExternalSources = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wwise SoundBank")
	bool bPackageAfterGeneration = true;
};
#pragma endregion

UCLASS()
class UNREALWAPPI_API UTESTDATA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wwise Infomation")
	FString WwiseVersion;
	UFUNCTION(CallInEditor, Category = "Wwise Infomation", meta = (FullyExpand = true))
	void CheckWwiseInfomation();

#pragma region Select

	UFUNCTION(CallInEditor, Category = "Wwise Select", meta = (FullyExpand = true))
	void SelectionInfo();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wwise Select")
	TArray<FSelectionStruct> Selections;
#pragma endregion

#pragma region Rename

	UFUNCTION(CallInEditor, Category = "Wwise Rename", meta = (FullyExpand = true))
	void SelectRename();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wwise Rename")
	int32 RenameIndex;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wwise Rename")
	FString NewName;
#pragma endregion

	
};
