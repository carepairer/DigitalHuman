// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyPlayerController.generated.h"


class UUserWidget;
class UInteractive;
class UMyWebSocketMgr;

/**
 * 
 */
UCLASS()
class DIGITALHUMAN_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	AMyPlayerController();

	// 保存前端 UI 类的引用
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UUserWidget> interactiveWidgetCls;

	UPROPERTY()
	UInteractive* interactiveWidget;

	UPROPERTY()
	UMyWebSocketMgr* webSockMgr;

};
