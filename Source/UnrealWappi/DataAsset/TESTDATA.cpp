// Fill out your copyright notice in the Description page of Project Settings.


#include "TESTDATA.h"
#include "AkAudio/Public/AkWaapiClient.h"


//for Ak Name Space
#include <AK/WwiseAuthoringAPI/waapi.h>

#include "AkWaapiUtils.h"
#include "UObject/SavePackage.h"



#pragma region CheckWwiseInfomation

void UTESTDATA::CheckWwiseInfomation()
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
			WwiseVersion = VersionObject->GetStringField("displayName");
			UE_LOG(LogTemp, Display, TEXT("Connected to Wwise: %s"), *WwiseVersion);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Version object not found in result."));
		}
	}
}

#pragma endregion

#pragma region SelectionInfo
void UTESTDATA::SelectionInfo()
{
	TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
	TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> Result;

	TArray<TSharedPtr<FJsonValue>> ReturnFields;
	ReturnFields.Add(MakeShareable(new FJsonValueString(TEXT("id"))));
	ReturnFields.Add(MakeShareable(new FJsonValueString(TEXT("name"))));
	ReturnFields.Add(MakeShareable(new FJsonValueString(TEXT("path"))));
	ReturnFields.Add(MakeShareable(new FJsonValueString(TEXT("type"))));

	Options->SetArrayField(TEXT("return"), ReturnFields);

	bool bSuccess = FAkWaapiClient::Get()->Call(ak::wwise::ui::getSelectedObjects, Args, Options, Result);

	if (bSuccess && Result.IsValid())
	{
		Selections.Empty(); // 正确清空

		const TArray<TSharedPtr<FJsonValue>>* ObjectsArray;
		if (Result->TryGetArrayField(TEXT("objects"), ObjectsArray))
		{
			for (auto& Obj : *ObjectsArray)
			{
				TSharedPtr<FJsonObject> Object = Obj->AsObject();
				FString Name = Object->GetStringField(TEXT("name"));
				FString ID = Object->GetStringField(TEXT("id"));
				FString Path = Object->GetStringField(TEXT("path"));
				FString Type = Object->GetStringField(TEXT("type"));

				Selections.Add(FSelectionStruct(Name, ID, Path, Type));
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ak.wwise.ui.getSelectedObjects call failed."));
	}
}
#pragma endregion

#pragma region SelectRename

void UTESTDATA::SelectRename()
{
    // 验证索引有效性
    if (!Selections.IsValidIndex(RenameIndex))
    {
        UE_LOG(LogTemp, Error, TEXT("无效索引: %d (总选择数: %d)"), RenameIndex, Selections.Num());
        return;
    }

    // 验证新名称
    if (NewName.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("新名称不能为空"));
        return;
    }

    FSelectionStruct& SelectedItem = Selections[RenameIndex];
    
    // 准备WAAPI调用
    TSharedRef<FJsonObject> Args = MakeShared<FJsonObject>();
    TSharedRef<FJsonObject> Options = MakeShared<FJsonObject>();
    TSharedPtr<FJsonObject> Result;

    // 正确构建参数结构 - 参考您提供的脚本中的标准方式
    Args->SetStringField("object", SelectedItem.ID);  // 设置要重命名的对象ID
    Args->SetStringField("value", NewName);           // 设置新名称

    // 检查WAAPI连接 - 参考您提供的脚本中的连接检查
    FAkWaapiClient* Client = FAkWaapiClient::Get();
    if (!Client || !Client->IsConnected())
    {
        UE_LOG(LogTemp, Error, TEXT("WAAPI连接未建立"));
        return;
    }

    // 执行重命名 - 参考您提供的脚本中的调用方式
    bool bSuccess = Client->Call(ak::wwise::core::object::setName, Args, Options, Result);

    if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("成功将'%s'(ID:%s)重命名为'%s'"), 
               *SelectedItem.Name, *SelectedItem.ID, *NewName);
        
        // 更新本地数据
        SelectedItem.Name = NewName;
        
        // 可选：刷新选择信息
        SelectionInfo();
    }
    else
    {
        FString ErrorMsg = TEXT("未知错误");
        if (Result.IsValid() && Result->HasField("message"))
        {
            ErrorMsg = Result->GetStringField("message");
        }
        UE_LOG(LogTemp, Error, TEXT("重命名失败: %s"), *ErrorMsg);
        
        // 调试输出实际发送的参数 - 参考您提供的脚本中的调试技术
        FString ArgsStr;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ArgsStr);
        FJsonSerializer::Serialize(Args, Writer);
        UE_LOG(LogTemp, Warning, TEXT("发送的参数: %s"), *ArgsStr);
    }
}



#pragma endregion

