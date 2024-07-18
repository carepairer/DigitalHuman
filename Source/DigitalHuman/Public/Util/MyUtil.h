// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class DIGITALHUMAN_API MyUtil
{
public:
	MyUtil();
	~MyUtil();

	UFUNCTION(BlueprintCallable, Category = "Image Utilities")
	static bool LoadImageToTexture(FString& ImgPath, TObjectPtr<UTexture2D>& Texture2D);


	//通过通配符查找文件，默认是匹配第一个，如果要找到匹配的第 n 个文件，
	//需要迭代所有匹配的文件，直到找到第 n 个为止
	UFUNCTION(BlueprintCallable, Category = "Image Utilities")
	static bool FindMatchingFile(FString& WildcardPath, FString& OutFoundPath);

	UFUNCTION(BlueprintCallable)
	static FLinearColor OpenColorPickerDialog();

	UFUNCTION(BlueprintCallable)
	static FString SelDirPath();

	static FString SelFilePath();

	static int32 GetImgNumFromPath(const FString& Path, const TArray<FString>& ImgExt);

	static int32 GetMinValue(std::initializer_list<int32> values);



	//================================================ 尚未使用 =========================== 
	//弹出虚拟键盘
	static bool ShowVirKeyboard();

	TArray<FString> GetImageFiles(const FString& Directory);

	// 解析字符串，提取前面的数字并转换为 int32 类型,遇到第一个非数字字符时停止。
	int32 ExtractNumFromStr(const FString& InputString);

public:
};
