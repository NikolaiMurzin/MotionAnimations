// Copyright 2023 Nikolai Anatolevich Murzin. All Rights Reserved.

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
