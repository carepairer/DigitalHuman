// Fill out your copyright notice in the Description page of Project Settings.


#include "Ctrl/MyPlayerController.h"
#include "UI/Interactive.h"
#include "WebSocket/MyWebSocketMgr.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include <Windows.h>
#include <commdlg.h>
#include "Windows/HideWindowsPlatformTypes.h"
#endif


AMyPlayerController::AMyPlayerController()
{
    // 设置要加载的 UI 类
    ConstructorHelpers::FClassFinder<UInteractive> WidgetClassFinder(TEXT("/Game/_Game/BP/UI/WBP_Interactive"));
    if (WidgetClassFinder.Succeeded())
    {
        interactiveWidgetCls = WidgetClassFinder.Class;
    }
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    interactiveWidget = CreateWidget<UInteractive>(this, interactiveWidgetCls);
    webSockMgr = CreateWidget<UMyWebSocketMgr>(GetWorld(), UMyWebSocketMgr::StaticClass());


    if (!interactiveWidget || !webSockMgr) {
        // 弹出消息框
        MessageBox(NULL, TEXT("类实例创建错误"), TEXT("运行错误"), MB_OK | MB_ICONWARNING);
        return;
    }

    // 显示鼠标光标
    bShowMouseCursor = true;

    // 锁定鼠标
    FInputModeGameAndUI InputMode;
    SetInputMode(InputMode);


    interactiveWidget->AddToViewport(0);
}