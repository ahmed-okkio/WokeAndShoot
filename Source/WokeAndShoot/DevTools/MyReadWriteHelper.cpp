// Fill out your copyright notice in the Description page of Project Settings.

#include "MyReadWriteHelper.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

MyReadWriteHelper::MyReadWriteHelper()
{
}

MyReadWriteHelper::~MyReadWriteHelper()
{
}
FString MyReadWriteHelper::LoadFileToString(FString FileName, FString FileArea) 
{
    FString Directory = FPaths::ProjectContentDir()+ "/" + FileArea;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FString Result = TEXT("No Result");

    if(PlatformFile.CreateDirectory(*Directory))
    {
        FString FilePath = Directory + "/"+ FileName;

        if(!FFileHelper::LoadFileToString(Result, *FilePath))
        {
            FFileHelper::SaveStringToFile(TEXT("PlayerName"),*FilePath);
            FFileHelper::LoadFileToString(Result, *FilePath);
            
            return Result;
        }
    }

    return Result;
}