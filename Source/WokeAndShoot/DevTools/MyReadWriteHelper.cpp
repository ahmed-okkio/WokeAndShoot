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
FString MyReadWriteHelper::LoadFileToString(FString& FilePath) 
{
    FString Directory = FPaths::ProjectContentDir() + FilePath;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FString Result = TEXT("No Result");

    if(PlatformFile.CreateDirectory(*Directory))
    {
        if(!FFileHelper::LoadFileToString(Result, *Directory))
        {
            FFileHelper::SaveStringToFile(TEXT("PlayerName"),*Directory);
            FFileHelper::LoadFileToString(Result, *Directory);
            return Result;
        }
    }

    return Result;
}

void MyReadWriteHelper::SaveStringToFile(FString& String, FString& FilePath) 
{
    FString Directory = FPaths::ProjectContentDir() + FilePath;
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    if(PlatformFile.CreateDirectory(*Directory))
    {
        FFileHelper::SaveStringToFile(String,*Directory);
    }
}