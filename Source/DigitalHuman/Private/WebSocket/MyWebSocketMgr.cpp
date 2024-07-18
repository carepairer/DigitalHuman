// Fill out your copyright notice in the Description page of Project Settings.


#include "WebSocket/MyWebSocketMgr.h"

#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Ctrl/MyPlayerController.h"

#include <Windows.h>
#include <debugapi.h>

void UMyWebSocketMgr::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    // 获取并转换PlayerController
    APlayerController* BaseController = GetOwningPlayer();
    if (BaseController)
    {
        myCtrl = Cast<AMyPlayerController>(BaseController);
    }

    serverURL = TEXT("ws://127.0.0.1:6017");
    StartWebSocketServer();
}

void UMyWebSocketMgr::NativeConstruct()
{
    Super::NativeConstruct();
}

void UMyWebSocketMgr::StartWebSocketServer()
{
    if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
    {
        FModuleManager::Get().LoadModule("WebSockets");
    }

    WebSocket = FWebSocketsModule::Get().CreateWebSocket(serverURL);

    WebSocket->OnMessage().AddUObject(this, &UMyWebSocketMgr::OnMessageReceived);
    WebSocket->OnClosed().AddUObject(this, &UMyWebSocketMgr::OnConnectionClosed);

    WebSocket->Connect();
}

void UMyWebSocketMgr::OnMessageReceived(const FString& Message)
{
    UE_LOG(LogTemp, Log, TEXT("Received message: %s"), *Message);
    
    // 输出到 Visual Studio 调试控制台
    OutputDebugStringW(*Message);
  
}

void UMyWebSocketMgr::OnConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
    UE_LOG(LogTemp, Warning, TEXT("WebSocket closed: %s"), *Reason);
}