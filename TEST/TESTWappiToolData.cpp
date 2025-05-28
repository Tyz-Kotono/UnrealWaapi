// Fill out your copyright notice in the Description page of Project Settings.


#include "TESTWappiToolData.h"
#include "AkAudio/Public/AkWaapiClient.h"


//for Ak Name Space
#include <AK/WwiseAuthoringAPI/waapi.h>

#include "AkWaapiUtils.h"
#include "UObject/SavePackage.h"


void UTESTWappiToolData::UpdateWwiseData()
{
	{
		//Begin undo group
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::undo::beginGroup, args, options, result);
	}

	//Create Switch GameSyncs
	CreateSwitchGroup("EnemyType", GetEnemyDataMembers(EnemyTypesArray), EnemyTypeSwitchGroupPath, EnemyTypeSwitchesPaths);
	CreateSwitchGroup("EnemyRarity", GetEnemyDataMembers(EnemyRaritiesArray), EnemyRaritySwitchGroupPath, EnemyRaritySwitchesPaths);
	CreateSwitchGroup("EnemyAction", GetEnemyDataMembers(EnemyActionsArray), EnemyActionsSwitchGroupPath, EnemyActionsSwitchesPaths);
	
	//Create Switch Containers Hierarchy
	CreateSwitchContainerStructure();

	{
		//End undo group
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("displayName", "Create Switch Hierarchy and necessary game syncs");
		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::undo::endGroup, args, options, result);
	}

	if (ClearPropertiesAfterSettingValues)
	{
		ClearWwiseProperties();
	}

	//TODO: Is this added to source control?
	SaveToAsset(this);
}

bool UTESTWappiToolData::SaveToAsset(UObject* ObjectToSave)
{
	UPackage* Package = ObjectToSave->GetPackage();
	const FString PackageName = Package->GetName();
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;

	// This is specified just for example
	{
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;
	}

	const bool bSucceeded = UPackage::SavePackage(Package, nullptr, *PackageFileName, SaveArgs);

	if (!bSucceeded)
	{
		UE_LOG(LogTemp, Error, TEXT("Package '%s' wasn't saved!"), *PackageName)
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Package '%s' was successfully saved"), *PackageName)
	return true;
}

void UTESTWappiToolData::ClearWwiseProperties()
{
	MainSwitchContainerOptionalProperties.Empty();

	for (int i = 0; i < EnemyTypesArray.Num(); i++)
	{
		EnemyTypesArray[i].WwiseOptionalProperties.Empty();
	}

	for (int i = 0; i < EnemyRaritiesArray.Num(); i++)
	{
		EnemyRaritiesArray[i].WwiseOptionalProperties.Empty();
	}

	for (int i = 0; i < EnemyActionsArray.Num(); i++)
	{
		EnemyActionsArray[i].WwiseOptionalProperties.Empty();
	}

	/*for (auto enemyType : EnemyTypesArray)
	{
		enemyType.WwiseOptionalProperties.Empty();
	}

	for (auto enemyRarity : EnemyRaritiesArray)
	{
		enemyRarity.WwiseOptionalProperties.Empty();
	}

	for (auto enemyAction : EnemyActionsArray)
	{
		enemyAction.WwiseOptionalProperties.Empty();
	}*/
}


void UTESTWappiToolData::UpdateEnemyTypeSwitch() //UNUSED ATM
{
	//Create or Update switches for EnemyType
	FString enumName = UEnum::GetValueAsString(EnemyTypeEnum::Last);
	enumName = enumName.Left(enumName.Find("::"));
	TArray<FString> switches;

	for (int i = 0; i < (int)EnemyTypeEnum::Last; i++)
	{
		EnemyTypeEnum type = (EnemyTypeEnum)i;
		switches.Emplace(EnumToString(type));
	}

	//CreateSwitchGroup(enumName, switches);
}

template<typename T>
TArray <FString> UTESTWappiToolData::GetEnemyDataMembers(T structToProcess)
{

	TArray<FString> SwitchNames;
	for (auto type : structToProcess)
	{
		if (!type.UpdateWwiseStructure)
		{
			continue;
		}

		SwitchNames.Emplace(type.Name);
	}

	return SwitchNames;
}

void UTESTWappiToolData::CreateSwitchContainerStructure()
{
	FString SwitchContainerPath = "";

	//Create Parent Switch Container. I'm just passing the first switch member as the default but maybe we could ask the user somehow.
	FString SwitchContainerID;
	SwitchContainerPath = CreateSwitchContainer(MainSwitchContainerParentPath, MainSwitchContainerName, EnemyTypeSwitchGroupPath, EnemyTypeSwitchesPaths[0], SwitchContainerID);

	if (MainSwitchContainerOptionalProperties.Num() > 0)
	{
		for (auto property : MainSwitchContainerOptionalProperties)
		{
			ApplyPropertiesToWwiseObjects(property, SwitchContainerPath);
		}
	}

	if (SetEnemyLevelAsRTPC)
	{
		auto rtpcPath = CreateRTPC(GameSyncsRTPC_ParentPath, "EnemyXPLevel", 1, 20, 1);
		SetRTPC(SwitchContainerPath, rtpcPath, "Volume");
	}

	{
		//Create children switch containers. We assume at least one enemy type.
		for (int i = 0; i < EnemyTypesArray.Num(); i++)
		{
			if (!EnemyTypesArray[i].UpdateWwiseStructure) 
			{
				continue;
			}

			//We create a switch container for each enemy type.
			FString switchTypeID;
			auto switchTypePath = CreateSwitchContainer(SwitchContainerPath, EnemyTypesArray[i].Name, EnemyRaritySwitchGroupPath, EnemyRaritySwitchesPaths[0], switchTypeID);

			if (EnemyTypesArray[i].WwiseOptionalProperties.Num() > 0)
			{
				for (auto property : EnemyTypesArray[i].WwiseOptionalProperties)
				{
					ApplyPropertiesToWwiseObjects(property, switchTypePath);
				}
			}

			auto AudioFileSufixType = MainSwitchContainerName;

			//Add switch assignments based on enemy type
			AddSwitchAssignment(SwitchContainerPath, switchTypeID, switchTypePath, EnemyTypeSwitchesPaths[i]);

			AudioFileSufixType = MainSwitchContainerName + "_" + EnemyTypesArray[i].Name;

			for (int j = 0; j < EnemyRaritiesArray.Num(); j++)
			{
				if (!EnemyRaritiesArray[j].UpdateWwiseStructure)
				{
					continue;
				}

				auto AudioFileSufixRarity = AudioFileSufixType;

				//We create a switch container for each enemy rarity
				FString switchRarityID;
				auto switchRarityPath = CreateSwitchContainer(switchTypePath, EnemyRaritiesArray[j].Name, EnemyActionsSwitchGroupPath, EnemyActionsSwitchesPaths[0], switchRarityID);

				if (EnemyRaritiesArray[j].WwiseOptionalProperties.Num() > 0)
				{
					for (auto property : EnemyRaritiesArray[j].WwiseOptionalProperties)
					{
						ApplyPropertiesToWwiseObjects(property, switchRarityPath);
					}
				}

				//Add switch assignments based on enemy rarity
				AddSwitchAssignment(switchTypePath, switchRarityID, switchRarityPath, EnemyRaritySwitchesPaths[j]);

				AudioFileSufixRarity = AudioFileSufixType + "_" + EnemyRaritiesArray[j].Name;

				for (int k = 0; k < EnemyActionsArray.Num(); k++)
				{
					if (!EnemyActionsArray[k].UpdateWwiseStructure)
					{
						continue;
					}

					{
						//We create the Sound SFX Objects now:
						FString SoundFXObjectID;
						auto SoundSFXObjectPath = CreateSoundSFX(switchRarityPath, EnemyActionsArray[k].Name, SoundFXObjectID);

						if (EnemyActionsArray[k].WwiseOptionalProperties.Num() > 0)
						{
							for (auto property : EnemyActionsArray[k].WwiseOptionalProperties)
							{
								ApplyPropertiesToWwiseObjects(property, SoundSFXObjectPath);
							}
						}

						//Add assignments to Sound SFXs based on Actions
						AddSwitchAssignment(switchRarityPath, SoundFXObjectID, SoundSFXObjectPath, EnemyActionsSwitchesPaths[k]);

						auto AudioFileSufixAction = AudioFileSufixRarity + "_" + EnemyActionsArray[k].Name;
						ImportAudio(SoundSFXObjectPath, AudioFilesSourceFolder + "\\" + AudioFileSufixAction + ".wav", AudioFilesOriginalsFolder);
					}
				}
			}
		}
	}
}

FString UTESTWappiToolData::CreateSoundSFX(FString ParentPath, FString SoundSFXName, FString& OutSoundFXID)
{
	FString SoundSFXPath = "";

	{
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

		//Parent Object
		TSharedRef<FJsonObject> parentObj = MakeShareable(new FJsonObject());
		parentObj->SetStringField("object", ParentPath);

		//Sound SFX Settings
		TSharedRef<FJsonObject> soundSFX = MakeShareable(new FJsonObject());
		soundSFX->SetStringField("type", "Sound");
		soundSFX->SetStringField("name", SoundSFXName);
		TSharedRef<FJsonValueObject> soundSFXValueObject = MakeShareable(new FJsonValueObject(soundSFX));

		//Sound SFXs Container Array
		TArray<TSharedPtr<FJsonValue>> soundSFXContainerArray;
		soundSFXContainerArray.Add(soundSFXValueObject);
		parentObj->SetArrayField("children", soundSFXContainerArray);

		//Workunits array
		TSharedRef<FJsonValueObject> ParentObjectValues = MakeShareable(new FJsonValueObject(parentObj));

		TArray<TSharedPtr<FJsonValue>> argsArray;
		argsArray.Add(ParentObjectValues);

		args->SetArrayField("objects", argsArray);
		args->SetStringField("onNameConflict", "merge");
		args->SetStringField("listMode", "append");

		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("path")), MakeShareable(new FJsonValueString("id"))};
		options->SetArrayField("return", optionsArray);
		TSharedPtr<FJsonObject> result;

		if (FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result))
		{
			auto resultObjs = result->GetArrayField("objects");
			auto childrenObj = resultObjs[0]->AsObject()->GetArrayField("children");
			SoundSFXPath = childrenObj[0]->AsObject()->GetStringField("path");
			OutSoundFXID = childrenObj[0]->AsObject()->GetStringField("id");
		}
	}

	return SoundSFXPath;
}

void UTESTWappiToolData::ImportAudio(FString ParentSoundSFX, FString FileLocation, FString OriginalsLocation)
{
	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

	//Sound SFX
	TSharedPtr<FJsonObject> SoundSFX = MakeShared<FJsonObject>();
	SoundSFX->SetStringField("object", ParentSoundSFX);

	//AudioFileSource Object
	TSharedPtr<FJsonObject> AudioFileSource = MakeShared<FJsonObject>();
	AudioFileSource->SetStringField("type", "AudioFileSource");
	AudioFileSource->SetStringField("name", "mySourceOne"); //How should I call this? Does it matter? How can I call it like the audio file name?

	//Files to import
	TSharedPtr<FJsonObject> files = MakeShared<FJsonObject>();

	//File One
	TSharedPtr<FJsonObject> fileOne = MakeShared<FJsonObject>();
	fileOne->SetStringField("audioFile", FileLocation);
	fileOne->SetStringField("originalsSubFolder", OriginalsLocation);
	TSharedRef<FJsonValueObject> fileOneValueObject = MakeShareable(new FJsonValueObject(fileOne));

	//Make Files Array
	TArray<TSharedPtr<FJsonValue>> filesArray;
	filesArray.Add(fileOneValueObject);
	files->SetArrayField("files", filesArray);

	AudioFileSource->SetObjectField("import", files);
	TSharedRef<FJsonValueObject> sourceValueObject = MakeShareable(new FJsonValueObject(AudioFileSource));

	//Make Sources Array
	TArray<TSharedPtr<FJsonValue>> sourcesArray;
	sourcesArray.Add(sourceValueObject);
	SoundSFX->SetArrayField("children", sourcesArray);
	TSharedRef<FJsonValueObject> soundValueObject = MakeShareable(new FJsonValueObject(SoundSFX));

	TArray<TSharedPtr<FJsonValue>> argsArray;
	argsArray.Add(soundValueObject);

	args->SetArrayField("objects", argsArray);
	args->SetStringField("onNameConflict", "merge");
	args->SetStringField("listMode", "append");

	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();

	TSharedPtr<FJsonObject> result;

	FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result);
}

FString UTESTWappiToolData::CreateSwitchContainer(FString ParentPath, FString ContainerName, FString SwitchGroup, FString Default, FString& OutID)
{
	FString SwitchContainerPath = "";

	{
		//Create Switch Container
		auto args = CreateSwitchContainerArgument(ParentPath, ContainerName);
		args->SetStringField("onNameConflict", "merge");
		args->SetStringField("listMode", "append");
		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("path")), MakeShareable(new FJsonValueString("id"))};
		options->SetArrayField("return", optionsArray);
		TSharedPtr<FJsonObject> result;

		if (FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result))
		{
			auto resultObjs = result->GetArrayField("objects");
			auto childrenObj = resultObjs[0]->AsObject()->GetArrayField("children");
			SwitchContainerPath = childrenObj[0]->AsObject()->GetStringField("path");
			OutID = childrenObj[0]->AsObject()->GetStringField("id");
		}
	}

	{
		//Set Switch Group Reference 
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("object", SwitchContainerPath);
		args->SetStringField("reference", "SwitchGroupOrStateGroup");
		args->SetStringField("value", SwitchGroup);
		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::object::setReference, args, options, result);
	}

	{
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("object", SwitchContainerPath);
		args->SetStringField("reference", "DefaultSwitchOrState");
		args->SetStringField("value", Default);
		TSharedRef<FJsonObject> options = MakeShareable(new FJsonObject());
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::object::setReference, args, options, result);
	}
	return SwitchContainerPath;
}

TSharedRef<FJsonObject> UTESTWappiToolData::CreateSwitchContainerArgument(FString parentPath, FString containerName)
{
	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

	//Parent WorkUnit
	TSharedRef<FJsonObject> workUnit = MakeShareable(new FJsonObject());
	workUnit->SetStringField("object", parentPath);

	//Switch Container Settings
	TSharedRef<FJsonObject> switchContainer = MakeShareable(new FJsonObject());
	switchContainer->SetStringField("type", "SwitchContainer");	
	switchContainer->SetStringField("name", containerName);
	TSharedRef<FJsonValueObject> switchContainerValueObject = MakeShareable(new FJsonValueObject(switchContainer));

	//Switch Container Array
	TArray<TSharedPtr<FJsonValue>> switchContainerArray;
	switchContainerArray.Add(switchContainerValueObject);
	workUnit->SetArrayField("children", switchContainerArray);

	//Workunits array
	TSharedRef<FJsonValueObject> ObjectValues = MakeShareable(new FJsonValueObject(workUnit));

	TArray<TSharedPtr<FJsonValue>> argsArray;
	argsArray.Add(ObjectValues);

	args->SetArrayField("objects", argsArray);
	return args;
}

void UTESTWappiToolData::CreateSwitchGroup(FString switchGroupName, TArray<FString> switchNames,
	FString& OutSwitchGroupPath, TArray<FString>& OutSwitchesPathsArray)
{
	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

	//Enemy Switches Workunit
	TSharedRef<FJsonObject> workUnit = MakeShareable(new FJsonObject());
	workUnit->SetStringField("object", GameSyncsSwitchesParentPath);

	//Switch Group
	TSharedRef<FJsonObject> switchGroup = MakeShareable(new FJsonObject());
	switchGroup->SetStringField("type", "SwitchGroup");
	switchGroup->SetStringField("name", switchGroupName);

	//Switch Group Children Array
	TArray<TSharedPtr<FJsonValue>> switchGroupChildrenArray;
	for (auto switchName : switchNames) 
	{
		switchGroupChildrenArray.Add(CreateSwitchJSONObject(switchName));
	}

	switchGroup->SetArrayField("children", switchGroupChildrenArray);

	TSharedRef<FJsonValueObject> switchGroupValueObject = MakeShareable(new FJsonValueObject(switchGroup));

	//Switch Group Array
	TArray<TSharedPtr<FJsonValue>> switchGroupArray;
	switchGroupArray.Add(switchGroupValueObject);
	workUnit->SetArrayField("children", switchGroupArray);

	TSharedRef<FJsonValueObject> ObjectValues = MakeShareable(new FJsonValueObject(workUnit));

	TArray<TSharedPtr<FJsonValue>> argsArray;
	argsArray.Add(ObjectValues);

	args->SetArrayField("objects", argsArray);
	args->SetStringField("onNameConflict", "merge"); 
	//Since the above is "merge" it doesn't destroy switches that are already there which is necessary
	//so we don't lose switch association that were already there. The downside is that if we remove
	//members from an enum, these won't be remove from the corresponding Wwise switch but I can live
	//with that.
	args->SetStringField("listMode", "append");

	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("path"))};
	options->SetArrayField("return", optionsArray);

	TSharedPtr<FJsonObject> result;

	FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result);

	if (result) 
	{
		auto resultObjs = result->GetArrayField("objects");
		auto switchContainer = resultObjs[0]->AsObject()->GetArrayField("children"); //Always use first object in array because we only modified one object (the work unit)

		OutSwitchGroupPath = switchContainer[0]->AsObject()->GetStringField("path"); //Always use first object in array because we only created one switch container. 

		auto switches = switchContainer[0]->AsObject()->GetArrayField("children");
		for (auto switchChildOfMine : switches)
		{
			OutSwitchesPathsArray.Emplace(switchChildOfMine->AsObject()->GetStringField("path"));
		}

	}
}

TSharedRef<FJsonValueObject> UTESTWappiToolData::CreateSwitchJSONObject(FString switchName)
{
	TSharedRef<FJsonObject> switchGroupChild = MakeShareable(new FJsonObject());
	switchGroupChild->SetStringField(WwiseWaapiHelper::TYPE, "Switch");
	switchGroupChild->SetStringField(WwiseWaapiHelper::NAME, switchName);
	return MakeShareable(new FJsonValueObject(switchGroupChild));
}

void UTESTWappiToolData::AddSwitchAssignment(FString SwitchContainerToCheck, FString SwitchContainerChildID, FString SwitchContainerToAssignPath, FString Switch)
{
	bool MakeAssignments = true;

	{
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("id", SwitchContainerToCheck);

		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::switchContainer::getAssignments, args, options, result);

		if (result)
		{
			auto assignments = result->GetArrayField("return");

			for (auto assign : assignments)
			{
				if (assign->AsObject()->GetStringField("child") == SwitchContainerChildID)
				{
					MakeAssignments = false;
					break;
				}
			}
		}
	}

	if (MakeAssignments)
	{
		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("child", SwitchContainerToAssignPath);
		args->SetStringField("stateOrSwitch", Switch);
		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::switchContainer::addAssignment, args, options, result);
	}
}

void UTESTWappiToolData::ApplyPropertiesToWwiseObjects(FWwiseNumericalProperty Property, FString ObjectPath)
{
	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
	args->SetStringField(WwiseWaapiHelper::OBJECT, ObjectPath);

	auto propertyName = GetWwisePropertyName(Property.Name);

	if (propertyName == "Invalid")
	{
		return; //Nothing to set.
	}

	auto propertyTargerValue = 0.0f;

	if (Property.Mode == WwisePropertySetModeEnum::Additive)
	{
		propertyTargerValue = GetPropertyValue(ObjectPath, propertyName) + Property.Value;
	}
	else 
	{
		propertyTargerValue = Property.Value;
	}

	args->SetStringField(WwiseWaapiHelper::PROPERTY, propertyName);

	if (Property.Name == WwiseNumericalPropertyEnum::Lowpass ||
		Property.Name == WwiseNumericalPropertyEnum::Highpass ||
		Property.Name == WwiseNumericalPropertyEnum::Pitch)
	{
		args->SetNumberField(WwiseWaapiHelper::VALUE, (int)propertyTargerValue);
	}
	else 
	{
		args->SetNumberField(WwiseWaapiHelper::VALUE, propertyTargerValue);
	}

	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TSharedPtr<FJsonObject> result;

	FAkWaapiClient::Get()->Call(ak::wwise::core::object::setProperty, args, options, result);
}

FString UTESTWappiToolData::GetWwisePropertyName(WwiseNumericalPropertyEnum PropertyName)
{
	switch (PropertyName)
	{
		case WwiseNumericalPropertyEnum::Volume:
			return "Volume";
		case WwiseNumericalPropertyEnum::Lowpass:
			return "Lowpass";
		case WwiseNumericalPropertyEnum::Highpass:
			return "Highpass";
		case WwiseNumericalPropertyEnum::InitialDelay:
			return "InitialDelay";
		case WwiseNumericalPropertyEnum::GameAuxSendVolume:
			return "GameAuxSendVolume";
		case WwiseNumericalPropertyEnum::Pitch:
			return "Pitch";
		default:
			return "Invalid";
	}
}

void UTESTWappiToolData::SetRTPC(FString TargetObject, FString RTPCPath, FString Property)
{
	{
		//Check if any of the current RTPCs matches the one we are trying to set.
		FString WAQLPath = "\"" + TargetObject + "\"";

		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
		args->SetStringField("waql", WAQLPath);

		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("@RTPC.ControlInput.path")), MakeShareable(new FJsonValueString("@RTPC.PropertyName"))}; //Tricky
		options->SetArrayField("return", optionsArray);

		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::object::get, args, options, result);

		if (result)
		{
			TArray<TSharedPtr<FJsonValue>> resultArray = result->GetArrayField("return");
			auto rtpcs = resultArray[0]->AsObject()->GetArrayField("@RTPC.ControlInput.path");

			for (int i = 0; i < rtpcs.Num(); i++)
			{
				FString rtpcName;
				FString PropertyName;
				rtpcs[i]->TryGetString(rtpcName);
				if (rtpcName == RTPCPath) //We already have an RTPC using this game parameter so let's check if it uses the same property.
				{
					auto PropertyNames = resultArray[0]->AsObject()->GetArrayField("@RTPC.PropertyName");
					PropertyNames[i]->TryGetString(PropertyName);
					if (PropertyName == Property)
					{
						return; //An RTPC that uses the same game parameter and propery already exists on this object so let's not set it again.
					}
				} 
			}
		}
	}

	{
		//Set the RTPC on the target object

		TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

		//Switch Container Object
		TSharedRef<FJsonObject> SwitchContainer = MakeShareable(new FJsonObject());
		SwitchContainer->SetStringField("object", TargetObject);

		//RTPC Object
		TSharedRef<FJsonObject> RTPCObject = MakeShareable(new FJsonObject());
		RTPCObject->SetStringField("type", "RTPC");
		RTPCObject->SetStringField("name", "");

		//RTPC Curve Object
		TSharedRef<FJsonObject> RTPCCurveObject = MakeShareable(new FJsonObject());
		RTPCCurveObject->SetStringField("type", "Curve");

		//RTPC Curve Points Objects
		TSharedRef<FJsonObject> Point1 = MakeShareable(new FJsonObject());
		Point1->SetNumberField("x", 1);
		Point1->SetNumberField("y", 0);
		Point1->SetStringField("shape", "Linear");
		TSharedRef<FJsonValueObject> Point1ValueObject = MakeShareable(new FJsonValueObject(Point1));

		TSharedRef<FJsonObject> Point2 = MakeShareable(new FJsonObject());
		Point2->SetNumberField("x", 20);
		Point2->SetNumberField("y", 6);
		Point2->SetStringField("shape", "Linear");
		TSharedRef<FJsonValueObject> Point2ValueObject = MakeShareable(new FJsonValueObject(Point2));

		//RTPC Curve Points Array
		TArray<TSharedPtr<FJsonValue>> RTPCCurvePoints;
		RTPCCurvePoints.Add(Point1ValueObject);
		RTPCCurvePoints.Add(Point2ValueObject);

		//Set Curve Objects
		RTPCCurveObject->SetArrayField("points", RTPCCurvePoints);
		RTPCObject->SetObjectField("@Curve", RTPCCurveObject);
		RTPCObject->SetStringField("@PropertyName", Property);
		RTPCObject->SetStringField("@ControlInput", RTPCPath);

		//Final assignments
		TSharedRef<FJsonValueObject> RTPCObjectValue = MakeShareable(new FJsonValueObject(RTPCObject));
		TArray<TSharedPtr<FJsonValue>> RTPCArray;
		RTPCArray.Add(RTPCObjectValue);
		SwitchContainer->SetArrayField("@RTPC", RTPCArray);
		TSharedRef<FJsonValueObject> ObjectValues = MakeShareable(new FJsonValueObject(SwitchContainer));

		TArray<TSharedPtr<FJsonValue>> argsArray;
		argsArray.Add(ObjectValues);

		args->SetArrayField("objects", argsArray);
		args->SetStringField("onNameConflict", "merge");
		// This would add more and more copies of the RTPC to the object, that's way we check if it already exists at the start of this method.
		args->SetStringField("listMode", "append");

		TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
		//TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("id"))};
		//options->SetArrayField("return", optionsArray);

		TSharedPtr<FJsonObject> result;

		FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result);
	}
}

FString UTESTWappiToolData::CreateRTPC(FString ParentPath, FString RTPCName, int Min, int Max, int Default)
{
	FString RTPCPatch = "";

	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();

	//Enemy Switches Workunit
	TSharedRef<FJsonObject> workUnit = MakeShareable(new FJsonObject());
	workUnit->SetStringField("object", ParentPath);

	//Switch Group
	TSharedRef<FJsonObject> RTPCObject = MakeShareable(new FJsonObject());
	RTPCObject->SetStringField("type", "GameParameter");
	RTPCObject->SetStringField("name", RTPCName);
	RTPCObject->SetNumberField("@Max", Max);
	RTPCObject->SetNumberField("@Min", Min);
	RTPCObject->SetNumberField("@InitialValue", Default);

	TSharedRef<FJsonValueObject> RTPCObjectValue = MakeShareable(new FJsonValueObject(RTPCObject));

	//Switch Container Array
	TArray<TSharedPtr<FJsonValue>> RTPCObjectsArray;
	RTPCObjectsArray.Add(RTPCObjectValue);
	workUnit->SetArrayField("children", RTPCObjectsArray);

	//Workunits array
	TSharedRef<FJsonValueObject> ObjectValues = MakeShareable(new FJsonValueObject(workUnit));

	TArray<TSharedPtr<FJsonValue>> argsArray;
	argsArray.Add(ObjectValues);

	args->SetArrayField("objects", argsArray);
	args->SetStringField("onNameConflict", "merge");
	args->SetStringField("listMode", "append");

	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString("path")), MakeShareable(new FJsonValueString("id"))};
	options->SetArrayField("return", optionsArray);
	TSharedPtr<FJsonObject> result;

	if (FAkWaapiClient::Get()->Call(ak::wwise::core::object::set, args, options, result))
	{
		auto resultObjs = result->GetArrayField("objects");
		auto childrenObj = resultObjs[0]->AsObject()->GetArrayField("children");
		RTPCPatch = childrenObj[0]->AsObject()->GetStringField("path");
	}

	return RTPCPatch;
}

float UTESTWappiToolData::GetPropertyValue(FString ObjectPath, FString PropertyName)
{
	FString WAQLPath = "\"" + ObjectPath + "\"";

	TSharedRef<FJsonObject> args = MakeShared<FJsonObject>();
	args->SetStringField("waql", WAQLPath);

	TSharedRef<FJsonObject> options = MakeShared<FJsonObject>();
	TArray<TSharedPtr<FJsonValue>> optionsArray {MakeShareable(new FJsonValueString(PropertyName))};
	options->SetArrayField("return", optionsArray);

	TSharedPtr<FJsonObject> result;

	FAkWaapiClient::Get()->Call(ak::wwise::core::object::get, args, options, result);

	if (result)
	{
		TArray<TSharedPtr<FJsonValue>> resultArray = result->GetArrayField("return");
		return (float)resultArray[0]->AsObject()->GetNumberField(PropertyName);
	}

	return 0.0f;
}
