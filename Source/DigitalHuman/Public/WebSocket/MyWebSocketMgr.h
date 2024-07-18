// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MyWebSocketMgr.generated.h"

class IWebSocket;
class AMyPlayerController;

/**
 * 
 */
UCLASS()
class DIGITALHUMAN_API UMyWebSocketMgr : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeOnInitialized() override;
    virtual void NativeConstruct() override;

public:

    void StartWebSocketServer();
    void OnMessageReceived(const FString& Message);
    void OnConnectionClosed(int32 StatusCode, const FString& Reason, bool bWasClean);;


    TSharedPtr<IWebSocket> WebSocket;
    FString serverURL;

    UPROPERTY()
    TObjectPtr<AMyPlayerController> myCtrl;
	
};
