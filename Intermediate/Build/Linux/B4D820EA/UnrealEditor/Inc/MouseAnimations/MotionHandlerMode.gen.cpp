// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/GeneratedCppIncludes.h"
#include "MouseAnimations/Private/MotionHandlerMode.h"
PRAGMA_DISABLE_DEPRECATION_WARNINGS
void EmptyLinkFunctionForGeneratedCodeMotionHandlerMode() {}
// Cross Module References
	MOUSEANIMATIONS_API UEnum* Z_Construct_UEnum_MouseAnimations_Mode();
	UPackage* Z_Construct_UPackage__Script_MouseAnimations();
// End Cross Module References
	static FEnumRegistrationInfo Z_Registration_Info_UEnum_Mode;
	static UEnum* Mode_StaticEnum()
	{
		if (!Z_Registration_Info_UEnum_Mode.OuterSingleton)
		{
			Z_Registration_Info_UEnum_Mode.OuterSingleton = GetStaticEnum(Z_Construct_UEnum_MouseAnimations_Mode, Z_Construct_UPackage__Script_MouseAnimations(), TEXT("Mode"));
		}
		return Z_Registration_Info_UEnum_Mode.OuterSingleton;
	}
	template<> MOUSEANIMATIONS_API UEnum* StaticEnum<Mode>()
	{
		return Mode_StaticEnum();
	}
	struct Z_Construct_UEnum_MouseAnimations_Mode_Statics
	{
		static const UECodeGen_Private::FEnumeratorParam Enumerators[];
#if WITH_METADATA
		static const UECodeGen_Private::FMetaDataPairParam Enum_MetaDataParams[];
#endif
		static const UECodeGen_Private::FEnumParams EnumParams;
	};
	const UECodeGen_Private::FEnumeratorParam Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enumerators[] = {
		{ "Mode::X", (int64)Mode::X },
		{ "Mode::XInverted", (int64)Mode::XInverted },
		{ "Mode::Y", (int64)Mode::Y },
		{ "Mode::YInverted", (int64)Mode::YInverted },
	};
#if WITH_METADATA
	const UECodeGen_Private::FMetaDataPairParam Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enum_MetaDataParams[] = {
		{ "ModuleRelativePath", "Private/MotionHandlerMode.h" },
		{ "X.Name", "Mode::X" },
		{ "XInverted.Name", "Mode::XInverted" },
		{ "Y.Name", "Mode::Y" },
		{ "YInverted.Name", "Mode::YInverted" },
	};
#endif
	const UECodeGen_Private::FEnumParams Z_Construct_UEnum_MouseAnimations_Mode_Statics::EnumParams = {
		(UObject*(*)())Z_Construct_UPackage__Script_MouseAnimations,
		nullptr,
		"Mode",
		"Mode",
		Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enumerators,
		UE_ARRAY_COUNT(Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enumerators),
		RF_Public|RF_Transient|RF_MarkAsNative,
		EEnumFlags::None,
		(uint8)UEnum::ECppForm::EnumClass,
		METADATA_PARAMS(Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enum_MetaDataParams, UE_ARRAY_COUNT(Z_Construct_UEnum_MouseAnimations_Mode_Statics::Enum_MetaDataParams))
	};
	UEnum* Z_Construct_UEnum_MouseAnimations_Mode()
	{
		if (!Z_Registration_Info_UEnum_Mode.InnerSingleton)
		{
			UECodeGen_Private::ConstructUEnum(Z_Registration_Info_UEnum_Mode.InnerSingleton, Z_Construct_UEnum_MouseAnimations_Mode_Statics::EnumParams);
		}
		return Z_Registration_Info_UEnum_Mode.InnerSingleton;
	}
	struct Z_CompiledInDeferFile_FID_UnrealProjects_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h_Statics
	{
		static const FEnumRegisterCompiledInInfo EnumInfo[];
	};
	const FEnumRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h_Statics::EnumInfo[] = {
		{ Mode_StaticEnum, TEXT("Mode"), &Z_Registration_Info_UEnum_Mode, CONSTRUCT_RELOAD_VERSION_INFO(FEnumReloadVersionInfo, 1681744845U) },
	};
	static FRegisterCompiledInInfo Z_CompiledInDeferFile_FID_UnrealProjects_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h_2697496777(TEXT("/Script/MouseAnimations"),
		nullptr, 0,
		nullptr, 0,
		Z_CompiledInDeferFile_FID_UnrealProjects_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h_Statics::EnumInfo, UE_ARRAY_COUNT(Z_CompiledInDeferFile_FID_UnrealProjects_cplusplus503_Plugins_MouseAnimations_Source_MouseAnimations_Private_MotionHandlerMode_h_Statics::EnumInfo));
PRAGMA_ENABLE_DEPRECATION_WARNINGS
