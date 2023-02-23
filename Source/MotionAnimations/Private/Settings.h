// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Settings.generated.h"
/**
 * 
 */
USTRUCT()
struct FSettings
{
	GENERATED_BODY()
public:
	FSettings();
	~FSettings();

	UPROPERTY()
	bool IsInCustomRange;
	UPROPERTY()
	int UpperRange;
	UPROPERTY()
	int LowerRange;

	UPROPERTY()
	TMap<FString, FString> Keys;

	void SetDefaultSettings();
	bool Save();
	FString GetFilePath();
	bool LoadSettingsFromDisk();
};
