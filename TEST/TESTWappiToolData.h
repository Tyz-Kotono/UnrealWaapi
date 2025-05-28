// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TESTWappiToolData.generated.h"

/**
 * 
 */

//////////////////////////////////////////////////////////////////////////
// Enum To String (Source: https://forums.unrealengine.com/t/conversion-of-enum-to-string/337869/24)
// Usage Example:
//		FString EnumString = EnumToString( EnumValue );
//////////////////////////////////////////////////////////////////////////
template <typename T>
FString EnumToString(T EnumValue)
{
	static_assert(TIsUEnumClass<T>::Value, "'T' template parameter to EnumToString must be a valid UEnum");
	return StaticEnum<T>()->GetNameStringByValue((int64)EnumValue);
}

UENUM(BlueprintType) //Unused
enum class EnemyTypeEnum : uint8
{
	Fire,
	Ice,
	Shadow,
	Last UMETA(Hidden)
};

UENUM(BlueprintType) //Unused
enum class EnemyRarityEnum : uint8
{
	Common,
	Rare,
	Epic,
	Last UMETA(Hidden)
};

UENUM(BlueprintType)
enum class WwiseNumericalPropertyEnum : uint8
{
	Volume,
	Lowpass,
	Highpass,
	InitialDelay,
	GameAuxSendVolume,
	Pitch
};

UENUM(BlueprintType)
enum class WwisePropertySetModeEnum : uint8
{
	Override,
	Additive
};

UENUM(BlueprintType)
enum class RTPCTraits : uint8
{
	Level,
	Health,
	Curse
};

USTRUCT(BlueprintType)
struct FAddRTPCToObject //Unused
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 0))
	RTPCTraits EnemyTrait = RTPCTraits::Level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 2))
	WwiseNumericalPropertyEnum Property = WwiseNumericalPropertyEnum::Volume;
};

USTRUCT(BlueprintType)
struct FWwiseNumericalProperty
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 0))
	WwiseNumericalPropertyEnum Name = WwiseNumericalPropertyEnum::Volume;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 1))
	float Value = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 2))
	WwisePropertySetModeEnum Mode = WwisePropertySetModeEnum::Additive;
};

USTRUCT(BlueprintType)
struct FEnemyDataAbstract
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 0))
	FString Name = "";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 1))
	bool UpdateWwiseStructure = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite,
		meta = (DisplayPriority = 2, EditCondition = "UpdateWwiseStructure", EditConditionHides))
	TArray<FWwiseNumericalProperty> WwiseOptionalProperties;

	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (DisplayPriority = 2, EditCondition = "UpdateWwiseStructure", EditConditionHides, DisplayName = "Add RTPCs To Object"))
	//TArray<FAddRTPCToObject> AddRTPCToObject;
};

USTRUCT(BlueprintType)
struct FEnemyTypeStruct : public FEnemyDataAbstract
{
	GENERATED_BODY()
};

USTRUCT(BlueprintType)
struct FEnemyRarityStruct : public FEnemyDataAbstract
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 BaseTreasureValue = 0;
};

USTRUCT(BlueprintType)
struct FEnemyActionStruct : public FEnemyDataAbstract
{
	GENERATED_BODY()
};

UCLASS()
class UNREALWAPPI_API UTESTWappiToolData : public UDataAsset
{
	GENERATED_BODY()

public:
	
	UFUNCTION(CallInEditor, Category = "WwiseData", meta = (FullyExpand = true))
	void UpdateWwiseData();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Game Syncs", meta = (FullyExpand = true))
	FString GameSyncsSwitchesParentPath = "\\Switches\\Enemies";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Game Syncs")
	FString GameSyncsRTPC_ParentPath = "\\Game Parameters\\Default Work Unit\\Enemies";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Main Switch Container")
	FString MainSwitchContainerParentPath = "\\Actor-Mixer Hierarchy\\EnemySFXs";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Main Switch Container")
	FString MainSwitchContainerName = "Enemy_SFX";

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WwiseData|Main Switch Container")
	TArray<FWwiseNumericalProperty> MainSwitchContainerOptionalProperties;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Audio Files")
	FString AudioFilesSourceFolder = "C:\\Users\\dunbe\\Desktop\\Enemy_SFX_Renders";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Audio Files")
	FString AudioFilesOriginalsFolder = "EnemyAudio";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WwiseData|Options", meta = (TitleProperty = "Name"))
	bool SetEnemyLevelAsRTPC;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WwiseData|Options")
	bool ClearPropertiesAfterSettingValues = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyData",
		meta = (TitleProperty = "Name", DisplayName= "Enemy Elemental Type"))
	TArray<FEnemyTypeStruct> EnemyTypesArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyData",
		meta = (TitleProperty = "Name", DisplayName = "Enemy Rarity"))
	TArray<FEnemyRarityStruct> EnemyRaritiesArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "EnemyData",
		meta = (TitleProperty = "Name", DisplayName = "Enemy Actions"))
	TArray<FEnemyActionStruct> EnemyActionsArray;

private:
	FString EnemyTypeSwitchGroupPath;
	TArray<FString> EnemyTypeSwitchesPaths;

	FString EnemyRaritySwitchGroupPath;
	TArray<FString> EnemyRaritySwitchesPaths;

	FString EnemyActionsSwitchGroupPath;
	TArray<FString> EnemyActionsSwitchesPaths;

	void UpdateEnemyTypeSwitch();

	template <typename T>
	TArray<FString> GetEnemyDataMembers(T structToProcess);

	void CreateSwitchContainerStructure();

	TSharedRef<FJsonObject> CreateSwitchContainerArgument(FString parentPath, FString containerName);
	FString CreateSwitchContainer(FString ParentPath, FString ContainerName, FString SwitchGroup, FString Default,
	                              FString& OutID);

	FString CreateSoundSFX(FString ParentPath, FString SoundSFXName, FString& OutSoundFXID);
	void ImportAudio(FString ParentSoundSFX, FString FileLocation, FString OriginalsLocation);

	void CreateSwitchGroup(FString switchGroupName, TArray<FString> switchNames,
	                       FString& OutSwitchGroupPath, TArray<FString>& OutSwitchesPathsArray);

	TSharedRef<FJsonValueObject> CreateSwitchJSONObject(FString switchName);
	void AddSwitchAssignment(FString SwitchContainerToCheck, FString SwitchContainerChildID,
	                         FString SwitchContainerToAssignPath, FString Switch);

	void ApplyPropertiesToWwiseObjects(FWwiseNumericalProperty Property, FString ObjectPath);
	FString GetWwisePropertyName(WwiseNumericalPropertyEnum Property);

	void SetRTPC(FString TargetObject, FString RTPCPath, FString Property);
	FString CreateRTPC(FString ParentPath, FString RTPCName, int Min, int Max, int Default);

	float GetPropertyValue(FString ObjectPath, FString PropertyName);
	bool SaveToAsset(UObject* ObjectToSave);

	void ClearWwiseProperties();
};
