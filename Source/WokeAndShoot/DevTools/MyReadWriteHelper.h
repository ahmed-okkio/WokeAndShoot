// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class WOKEANDSHOOT_API MyReadWriteHelper
{
public:
	MyReadWriteHelper();
	~MyReadWriteHelper();

	static FString LoadFileToString(FString FileName,FString FileArea);

};
