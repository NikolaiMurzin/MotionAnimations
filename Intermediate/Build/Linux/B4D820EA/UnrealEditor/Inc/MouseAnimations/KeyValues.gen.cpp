// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MouseAnimations/Private/KeyValues.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeKeyValues() {}
// Cross Module References
	MOUSEANIMATIONS_API UScriptStruct* Z_Construct_UScriptStruct_FKeyValues();
	UPackage* Z_Construct_UPackage__Script_MouseAnimations();
	COREUOBJECT_API UScriptStruct* Z_Construct_UScriptStruct_FFrameNumber();
	MOVIESCENE_API UScriptStruct* Z_Construct_UScriptStruct_FMovieSceneFloatValue();
	MOVIESCENE_API UScriptStruct* Z_Construct_UScriptStruct_FMovieSceneDoubleValue();
// End Cross Module References
	static FStructRegistrationInfo Z_Registration_Info_UScriptStruct_KeyValues;
class UScriptStruct* FKeyValues::StaticStruct()
{
	if (!Z_Registration_Info_UScriptStruct_KeyValues.OuterSingleton)
	{
		Z_Registration_Info_UScriptStruct_KeyValues.OuterSingleton = GetStaticStruct(Z_Construct_UScriptStruct_FKeyValues, Z_Construct_UPackage__Script_MouseAnimations(), TEXT("KeyValues"));
	}
	return Z_Registration_Info_UScriptStruct_KeyValues.OuterSingleton;
}
template<> MOUSEANIMATIONS_API UScriptStruct* StaticStruct<FKeyValues>()
{
	return FKeyValues::StaticStruct();
}
	struct Z_Construct_UScriptStruct_FKeyValues_Statics
	{
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Struct_MetaDataParams[];
#endif
		static void* NewStructOps();
		static const UECodeGen_Private::FStructPropertyParams NewProp_Times_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_Times_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_Times;
		static const UECodeGen_Private::FStructPropertyParams NewProp_FloatValues_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_FloatValues_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_FloatValues;
		static const UECodeGen_Private::FStructPropertyParams NewProp_DoubleValues_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_DoubleValues_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_DoubleValues;
		static const UECodeGen_Private::FIntPropertyParams NewProp_IntegerValues_Inner;
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam NewProp_IntegerValues_MetaData[];
#endif
		static const UECodeGen_Private::FArrayPropertyParams NewProp_IntegerValues;
		static const UECodeGen_Private::FPropertyParamsBase* const PropPointers[];
		static const UECodeGen_Private::FStructParams ReturnStructParams;
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FKeyValues_Statics::Struct_MetaDataParams[] = {
		{ "ModuleRelativePath", "Private/KeyValues.h" },
	};
#endif
	void* Z_Construct_UScriptStruct_FKeyValues_Statics::NewStructOps()
	{
		return (UScriptStruct::ICppStructOps*)new UScriptStruct::TCppStructOps<FKeyValues>();
	}
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times_Inner = { "Times", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FFrameNumber, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times_MetaData[] = {
		{ "ModuleRelativePath", "Private/KeyValues.h" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times = { "Times", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FKeyValues, Times), EArrayPropertyFlags::None, METADATA_PARAMS(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times_MetaData)) };
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues_Inner = { "FloatValues", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FMovieSceneFloatValue, METADATA_PARAMS(nullptr, 0) }; // 2127412773
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues_MetaData[] = {
		{ "ModuleRelativePath", "Private/KeyValues.h" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues = { "FloatValues", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FKeyValues, FloatValues), EArrayPropertyFlags::None, METADATA_PARAMS(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues_MetaData)) }; // 2127412773
	const UECodeGen_Private::FStructPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues_Inner = { "DoubleValues", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Struct, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, Z_Construct_UScriptStruct_FMovieSceneDoubleValue, METADATA_PARAMS(nullptr, 0) }; // 1764133413
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues_MetaData[] = {
		{ "ModuleRelativePath", "Private/KeyValues.h" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues = { "DoubleValues", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FKeyValues, DoubleValues), EArrayPropertyFlags::None, METADATA_PARAMS(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues_MetaData)) }; // 1764133413
	const UECodeGen_Private::FIntPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues_Inner = { "IntegerValues", nullptr, (EPropertyFlags)0x0000000000000000, UECodeGen_Private::EPropertyGenFlags::Int, RF_Public|RF_Transient|RF_MarkAsNative, 1, 0, METADATA_PARAMS(nullptr, 0) };
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues_MetaData[] = {
		{ "ModuleRelativePath", "Private/KeyValues.h" },
	};
#endif
	const UECodeGen_Private::FArrayPropertyParams Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues = { "IntegerValues", nullptr, (EPropertyFlags)0x0010000000000000, UECodeGen_Private::EPropertyGenFlags::Array, RF_Public|RF_Transient|RF_MarkAsNative, 1, STRUCT_OFFSET(FKeyValues, IntegerValues), EArrayPropertyFlags::None, METADATA_PARAMS(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues_MetaData, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues_MetaData)) };
	const UECodeGen_Private::FPropertyParamsBase* const Z_Construct_UScriptStruct_FKeyValues_Statics::PropPointers[] = {
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_Times,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_FloatValues,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_DoubleValues,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues_Inner,
		(const UECodeGen_Private::FPropertyParamsBase*)&Z_Construct_UScriptStruct_FKeyValues_Statics::NewProp_IntegerValues,
	};
	const UECodeGen_Private::FStructParams Z_Construct_UScriptStruct_FKeyValues_Statics::ReturnStructParams = {
		(UObject* (*)())Z_Construct_UPackage__Script_MouseAnimations,
		nullptr,
		&NewStructOps,
		"KeyValues",
		sizeof(FKeyValues),
		alignof(FKeyValues),
		Z_Construct_UScriptStruct_FKeyValues_Statics::PropPointers,
		UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::PropPointers),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EStructFlags(0x00000001),
		METADATA_PARAMS(Z_Construct_UScriptStruct_FKeyValues_Statics::Struct_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UScriptStruct_FKeyValues_Statics::Struct_MetaDataParams))
	};
	UScriptStruct* Z_Construct_UScriptStruct_FKeyValues()
	{
		if (!Z_Registration_Info_UScriptStruct_KeyValues.InnerSingleton)
		{
			UECodeGen_Private::ConstructUScriptStruct(Z_Registration_Info_UScriptStruct_KeyValues.InnerSingleton, Z_Construct_UScriptStruct_FKeyValues_Statics::ReturnStructParams);
		}
		return Z_Registration_Info_UScriptStruct_KeyValues.InnerSingleton;
	}
	struct Z_CompiledInDeferFile_FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_KeyValues_h_Statics
	{
		static const FStructRegisterCompiledInInfo ScriptStructInfo[];
	};
	const FStructRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_KeyValues_h_Statics::ScriptStructInfo[] = {
		{ FKeyValues::StaticStruct, Z_Construct_UScriptStruct_FKeyValues_Statics::NewStructOps, TEXT("KeyValues"), &Z_Registration_Info_UScriptStruct_KeyValues, CONSTRUCT_RELOAD_VERSION_INFO(FStructReloadVersionInfo, sizeof(FKeyValues), 1262242742U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_KeyValues_h_2275643890(TEXT("/Script/MouseAnimations"),
		nullptr, 0,
		Z_CompiledInDeferFile_FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_KeyValues_h_Statics::ScriptStructInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UnrealProjects_AllOfTheLights503_Plugins_MouseAnimations_Source_MouseAnimations_Private_KeyValues_h_Statics::ScriptStructInfo),
		nullptr, 0);
PRAGMA_ENABLE_DEPRECATION_WARNINGS
