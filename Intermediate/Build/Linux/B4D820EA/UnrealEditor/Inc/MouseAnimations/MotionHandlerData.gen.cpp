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
	const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FMotionHandlerData_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_MouseAnimations,
		nullptr,
		&NewStructOps,
		"MotionHandlerData",
		sizeof(FMotionHandlerData),
		alignof(FMotionHandlerData),
		nullptr,
		0,
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
		{ FMotionHandlerData::StaticStruct, Z_Construct_UScriptStruct_FMotionHandlerData_Statics::NewStructOps, TEXT("MotionHandlerData"), &Z_Registration_Info_UScriptStruct_MotionHandlerData, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FMotionHandlerData), 3446134502U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_265645340(TEXT("/Script/MouseAnimations"),
		nullptr, 0,
		Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerData_h_Statics::ScriptStructInfo),
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
