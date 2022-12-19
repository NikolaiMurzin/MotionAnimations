// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MouseAnimations/Private/MotionHandlerData.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeMotionHandlerData() {}
// Cross Module References
	MOUSEANIMATIONS_API UScriptStruct* Z_Construct_UScriptStruct_FMotionHandlerData();
	UPackage* Z_Construct_UPackage__Script_MouseAnimations();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FGuid();
	MOUSEANIMATIONS_API UEnum* Z_Construct_UEnum_MouseAnimations_Mode();
	MOUSEANIMATIONS_API UScriptStruct* Z_Construct_UScriptStruct_FKeyValues();
// End Cross Module References
	static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_MotionHandlerData;
class UScriptStruct* FMotionHandlerData::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_MotionHandlerData.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_MotionHandlerData.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FMotionHandlerData, Z_Construct_UPackage__Script_MouseAnimations(), TEXT("MotionHandlerData"));
	}
	return Z_Registration_Info_UScriptStruct_MotionHandlerData.OuterSingleton;
}
template<> MOUSEANIMATIONS_API UScriptStruct* StaticStruct<FMotionHandlerData>()
{
	return FMotionHandlerData::StaticStruct();
}
	struct Z_Construct_UScriptStruct_FMotionHandlerData_Statics
	{
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Scale_MetaData[];
#endif
		static const UECodeGen_Private::FDoublePropertyParams NewProp_Scale;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_ObjectFGuid_MetaData[];
#endif
		static const UECodeGen_Private::FStructPropertyParams NewProp_ObjectFGuid;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_TrackName_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_TrackName;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SectionRowIndex_MetaData[];
#endif
		static const UECodeGen_Private::FIntPropertyParams NewProp_SectionRowIndex;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_ChannelTypeName_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_ChannelTypeName;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_ChannelIndex_MetaData[];
#endif
		static const UECodeGen_Private::FIntPropertyParams NewProp_ChannelIndex;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_ControlSelection_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_ControlSelection;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SequenceName_MetaData[];
#endif
		static const UECodeGen_Private::FStrPropertyParams NewProp_SequenceName;
		static const UECodeGen_Private::FBytePropertyParams NewProp_SelectedMode_Underlying;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_SelectedMode_MetaData[];
#endif
		static const UECodeGen_Private::FEnumPropertyParams NewProp_SelectedMode;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_CustomName_MetaData[];
#endif
		static const UECodeGen_Private::FTextPropertyParams NewProp_CustomName;
		static const UECodeGen_Private::FStructPropertyParams NewProp_KeyValues_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_KeyValues_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_KeyValues;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UECodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FMotionHandlerData>();
	}
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_Scale_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FDoublePropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_Scale = { "Scale", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Double, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, Scale), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_Scale_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_Scale_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ObjectFGuid_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ObjectFGuid = { "ObjectFGuid", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, ObjectFGuid), Z_Construct_UScriptStruct_FGuid, METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ObjectFGuid_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ObjectFGuid_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_TrackName_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_TrackName = { "TrackName", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, TrackName), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_TrackName_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_TrackName_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SectionRowIndex_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SectionRowIndex = { "SectionRowIndex", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, SectionRowIndex), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SectionRowIndex_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SectionRowIndex_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelTypeName_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelTypeName = { "ChannelTypeName", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, ChannelTypeName), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelTypeName_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelTypeName_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelIndex_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelIndex = { "ChannelIndex", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, ChannelIndex), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelIndex_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelIndex_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ControlSelection_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ControlSelection = { "ControlSelection", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, ControlSelection), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ControlSelection_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ControlSelection_MetaData)) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SequenceName_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FStrPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SequenceName = { "SequenceName", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Str, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, SequenceName), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SequenceName_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SequenceName_MetaData)) };
	const UECodeGen_Private::FBytePropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode_Underlying = { "UnderlyingType", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Byte, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, nullptr, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FEnumPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode = { "SelectedMode", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Enum, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, SelectedMode), Z_Construct_UEnum_MouseAnimations_Mode, METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode_MetaData)) }; // 1681744845
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_CustomName_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FTextPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_CustomName = { "CustomName", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Text, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, CustomName), METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_CustomName_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_CustomName_MetaData)) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues_Inner = { "KeyValues", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FKeyValues, METADATA_PARAMS(nullptr, 0) }; // 1262242742
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues_MetaData[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerData.h" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues = { "KeyValues", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FMotionHandlerData, KeyValues), EArrayPropertyFlags::None, METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues_MetaData)) }; // 1262242742
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FMotionHandlerData_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_Scale,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ObjectFGuid,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_TrackName,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SectionRowIndex,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelTypeName,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ChannelIndex,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_ControlSelection,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SequenceName,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode_Underlying,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_SelectedMode,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_CustomName,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewProp_KeyValues,
	};
	const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_MouseAnimations,
		nullptr,
		&NewStructOps,
		"MotionHandlerData",
		sizeof(FMotionHandlerData),
		alignof(FMotionHandlerData),
		Z_Construct_UScriptStruct_FMotionHandlerData_Statics::PropPointers,
		UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::Struct_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FMotionHandlerData_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FMotionHandlerData()
	{
		if (!Z_Registration_Info_UScriptStruct_MotionHandlerData.InnerSingleton)
		{
			UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_MotionHandlerData.InnerSingleton, Z_Construct_UScriptStruct_FMotionHandlerData_Statics::ReturnStructParams);
		}
		return Z_Registration_Info_UScriptStruct_MotionHandlerData.InnerSingleton;
	}
	struct Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics
	{
		static const FStructRegisterCompiledInInfo ScriptStructInfo[];
	};
	const FStructRegisterCompiledInInfo Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics::ScriptStructInfo[] = {
		{ FMotionHandlerData::StaticStruct, Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewStructOps, TEXT("MotionHandlerData"), &Z_Registration_Info_UScriptStruct_MotionHandlerData, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMotionHandlerData), 3385703598U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_3054230558(TEXT("/Script/MouseAnimations"),
		nullptr, 0,
		Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics::ScriptStructInfo),
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
