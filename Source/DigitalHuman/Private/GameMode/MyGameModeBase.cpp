// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/MyGameModeBase.h"
#include "Ctrl/MyPlayerController.h"

AMyGameModeBase::AMyGameModeBase()
{
	// 使用自定义的 Controller 类
	PlayerControllerClass = AMyPlayerController::StaticClass();

}
