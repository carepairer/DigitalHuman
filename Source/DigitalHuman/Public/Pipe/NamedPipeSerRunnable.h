// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <string>
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeBool.h"

// 前向声明 HANDLE，避免直接依赖 windows.h
typedef void* HANDLE;
class UInteractive;

/**
 * 
 */
class DIGITALHUMAN_API NamedPipeSerRunnable : public FRunnable
{
public:
    NamedPipeSerRunnable();
    virtual ~NamedPipeSerRunnable();

    NamedPipeSerRunnable(UInteractive* InteractiveInsTemp); // 构造函数中传递实例

    // FRunnable interface
    virtual bool Init() override;
    virtual uint32 Run() override;
    virtual void Stop() override;

    // 控制数据接收的方法
    void StartAcceptingData();
    void StopAcceptingData();


    UInteractive* InteractiveIns; // 保存对UInteractive实例的引用

private:
    HANDLE Pipe;
    FThreadSafeBool bAcceptingData;

    void HandleReceivedData(const FString& Data);
    void HandleBusinessLogic(const FString& Data);
    bool ConnectToPipe();
};
