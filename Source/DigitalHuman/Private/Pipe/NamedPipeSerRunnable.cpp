// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "Pipe/NamedPipeSerRunnable.h"

// 包含 windows.h 并处理潜在冲突
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

// 取消定义可能冲突的宏
#ifdef UpdateResource
#undef UpdateResource
#endif

#include "UI/Interactive.h"
#include <Components/ComboBoxString.h>
#include "Async/Async.h"


NamedPipeSerRunnable::NamedPipeSerRunnable() : Pipe(INVALID_HANDLE_VALUE), bAcceptingData(true)
{

}

NamedPipeSerRunnable::~NamedPipeSerRunnable()
{
    Stop();
}

NamedPipeSerRunnable::NamedPipeSerRunnable(UInteractive* InteractiveInsTemp)
    : Pipe(INVALID_HANDLE_VALUE), bAcceptingData(true), InteractiveIns(InteractiveInsTemp)
{

}

bool NamedPipeSerRunnable::Init()
{
    return ConnectToPipe();
}

uint32 NamedPipeSerRunnable::Run()
{
    while (true) // 无限循环，持续监听管道
    {
        if (Pipe == INVALID_HANDLE_VALUE)
        {
            ConnectToPipe(); // 尝试重新连接管道
        }

        char Buffer[1024];
        DWORD BytesRead;
        BOOL ReadResult = ReadFile(Pipe, Buffer, sizeof(Buffer) - 1, &BytesRead, nullptr);
        if (ReadResult && BytesRead > 0)
        {
            Buffer[BytesRead] = '\0'; // 确保缓冲区以空字符结尾
            Buffer[BytesRead + 1] = '\0'; // 确保以两个空字符结尾

            // 将字节数据转换为 FString
            FString ReceivedString = UTF8_TO_TCHAR((const char*)Buffer);

            // 处理接收到的字符串数据
            HandleReceivedData(ReceivedString);

            //// Echo back the message
            //FString Response = FString::Printf(TEXT("Echo: %s"), *ReceivedString);
            //DWORD BytesWritten;
            //WriteFile(Pipe, TCHAR_TO_UTF8(*Response), FCString::Strlen(*Response), &BytesWritten, nullptr); // 将响应转换为UTF-8发送回客户端
        }
        else
        {
            DWORD lastError = GetLastError();
            if (lastError == ERROR_BROKEN_PIPE)
            {
                UE_LOG(LogTemp, Warning, TEXT("Client disconnected"));
                CloseHandle(Pipe);
                Pipe = INVALID_HANDLE_VALUE;
            }
            else
            {
                // 记录其他错误，并保持管道打开状态
                //UE_LOG(LogTemp, Error, TEXT("Failed to read from named pipe: %d"), lastError);
                // 引入等待机制，避免忙等待
                FPlatformProcess::Sleep(0.5f);
            }
        }

        // 在循环末尾休眠一段时间，避免忙等待
        FPlatformProcess::Sleep(0.1f);
    }

    return 0;
}

void NamedPipeSerRunnable::Stop()
{
    if (Pipe != INVALID_HANDLE_VALUE)
    {
        CloseHandle(Pipe);
        Pipe = INVALID_HANDLE_VALUE;
    }
}

void NamedPipeSerRunnable::StartAcceptingData()
{
    //管道恢复接收数据
    bAcceptingData = true;
    UE_LOG(LogTemp, Log, TEXT("Started accepting data."));
}

void NamedPipeSerRunnable::StopAcceptingData()
{
    bAcceptingData = false;
}

bool NamedPipeSerRunnable::ConnectToPipe()
{
    Pipe = CreateNamedPipe(
        TEXT("\\\\.\\pipe\\MyNamedPipe"),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        1024 * 16,
        1024 * 16,
        0,
        nullptr);

    if (Pipe == INVALID_HANDLE_VALUE)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create named pipe: %d"), GetLastError());
        return false;
    }

    //UE_LOG(LogTemp, Log, TEXT("Named pipe server created, waiting for client to connect..."));
    return true;
}

void NamedPipeSerRunnable::HandleReceivedData(const FString& Data)
{
    if (bAcceptingData)
    {
        UE_LOG(LogTemp, Log, TEXT("Failed to read from named pipe: %s"), *Data);

        //开始拒收数据
        StopAcceptingData();
        HandleBusinessLogic(Data);
    }
    else
    {
        // 不接受数据时，直接丢弃
        UE_LOG(LogTemp, Log, TEXT("Ignoring received data: %s"), *Data);
        
    }
}

void NamedPipeSerRunnable::HandleBusinessLogic(const FString& Data)
{
    UE_LOG(LogTemp, Log, TEXT("HandleBusinessLogic: %s"), *Data);
    FString strCmbox = TEXT("");

    // 检查是否包含特定字符串并执行相应操作
    if (Data.Contains(TEXT("古汉传承令")) ||
        Data.Contains(TEXT("武汉传承令")) ||  
        Data.Contains(TEXT("武汉传承中影")) ||  //====== 预留选项 =======
        Data.Contains(TEXT("武汉传承令")) ||  //====== 预留选项 =======
        Data.Contains(TEXT("武汉传承令")) ||  //====== 预留选项 =======
        Data.Contains(TEXT("武汉传承令")) ||  //====== 预留选项 =======
        Data.Contains(TEXT("武汉传承令")) ||  //====== 预留选项 =======
        Data.Contains(TEXT("古汉传承令")))
    {
        strCmbox = TEXT("oppo");
    }
    else if (Data.Contains(TEXT("介绍整个景区")))
    {
        strCmbox = TEXT("jizhitangjiang");
    }
    else if (Data.Contains(TEXT("什么叫做辛小追")) || 
             Data.Contains(TEXT("什么叫做新小罪")) ||
             Data.Contains(TEXT("什么叫做新小锥")) || 
             Data.Contains(TEXT("什么叫做新小辉")) || 
             Data.Contains(TEXT("什么叫做心小椎")) ||
             Data.Contains(TEXT("什么叫做西小锥")) ||
             Data.Contains(TEXT("什么叫做星小队")) ||
             Data.Contains(TEXT("什么叫做七小尊")) || 
             Data.Contains(TEXT("什么叫做邢小辉")) || 
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")) || //====== 预留选项 =======
             Data.Contains(TEXT("什么叫做星小队")))
    {
        strCmbox = TEXT("lanjv");
    }
    else if (Data.Contains(TEXT("秘诀是什么")) ||
             Data.Contains(TEXT("拒觉是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("秘诀是什么")) 
            )
    {
        strCmbox = TEXT("wangwang");
    }
    else if (Data.Contains(TEXT("歧黄太学")) ||
             Data.Contains(TEXT("齐鹏太学")) ||
             Data.Contains(TEXT("吉华大学")) ||
             Data.Contains(TEXT("秦皇太玄")) ||
             Data.Contains(TEXT("齐房太玄")) ||
             Data.Contains(TEXT("秦皇快学")) ||
             Data.Contains(TEXT("齐黄大学")) ||
             Data.Contains(TEXT("秦皇派学")) ||
             Data.Contains(TEXT("秦皇太岁")) ||
             Data.Contains(TEXT("秦皇太选")) ||
             Data.Contains(TEXT("祁皇探寻")) ||
             Data.Contains(TEXT("齐彭太学")) ||
             Data.Contains(TEXT("秦皇太学")) ||
             Data.Contains(TEXT("秦皇太穴")) ||
             Data.Contains(TEXT("齐皇太学")) ||
             Data.Contains(TEXT("集合派学")) ||
             Data.Contains(TEXT("齐王太学")) ||
             Data.Contains(TEXT("祁皇探寻")) ||
             Data.Contains(TEXT("奇环快穴")) ||
             Data.Contains(TEXT("齐黄派穴")) ||
             Data.Contains(TEXT("期华太学")) ||
             Data.Contains(TEXT("西黄派雪")) ||
             Data.Contains(TEXT("岐黄害穴")) ||
             Data.Contains(TEXT("秦皇太岁")) ||
             Data.Contains(TEXT("奇葩乱穴")) ||
             Data.Contains(TEXT("急缓害学")) ||
             Data.Contains(TEXT("齐黄太学")) ||
             Data.Contains(TEXT("奇宝快学")) ||
             Data.Contains(TEXT("集团泰熊")) ||
             Data.Contains(TEXT("奇宝快学")) ||
             Data.Contains(TEXT("秦王快旋")) ||
             Data.Contains(TEXT("齐广派学")) ||
             Data.Contains(TEXT("齐皇快学")) ||
             Data.Contains(TEXT("齐冯爱学")) ||
             Data.Contains(TEXT("棋鹏太学")) ||
             Data.Contains(TEXT("期房爱学")) ||
             Data.Contains(TEXT("轻狂的爱学")) ||
             Data.Contains(TEXT("集团快讯")) ||
             Data.Contains(TEXT("祁皇太虚")) ||
             Data.Contains(TEXT("启逢开学")) ||
             Data.Contains(TEXT("齐光海穴")) ||
             Data.Contains(TEXT("旗袍海选")) ||
             Data.Contains(TEXT("秦黄爱荀")) ||
             Data.Contains(TEXT("起步排穴")) ||
             Data.Contains(TEXT("棋袍太循")) ||
             Data.Contains(TEXT("齐王害学")) ||
             Data.Contains(TEXT("秦黄海穴")) ||
             Data.Contains(TEXT("齐皇太玄")) ||
             Data.Contains(TEXT("奇法快学")) ||
             Data.Contains(TEXT("棋皇太水")) ||
             Data.Contains(TEXT("齐王爱学")) ||
             Data.Contains(TEXT("骑逢太虚")) ||
             Data.Contains(TEXT("齐王？爱学")) ||
             Data.Contains(TEXT("祁皇太玄")) ||  
             Data.Contains(TEXT("祁皇太穴")) ||  
             Data.Contains(TEXT("齐王快学")) ||  
             Data.Contains(TEXT("秦黄派雪")) ||  
             Data.Contains(TEXT("棋皇太学")) ||  
             Data.Contains(TEXT("什么要做秦皇太子")) ||  
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("西黄派雪")) ||  //====== 预留选项 =======
             Data.Contains(TEXT("岐黄害穴"))     
             )
    {
        strCmbox = TEXT("oupaoguonai");
    }
    else if (Data.Contains(TEXT("介绍研学中心")) ||
             Data.Contains(TEXT("介绍营修中心")) ||    
             Data.Contains(TEXT("介绍文学中心")) ||    
             Data.Contains(TEXT("介绍研究中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍营修中心")) ||    //====== 预留选项 =======
             Data.Contains(TEXT("介绍文学中心")))
    {
        strCmbox = TEXT("mengniusuansuanru");
    }
    else if (Data.Contains(TEXT("中医知识问答")))
    {
        strCmbox = TEXT("wahahalongjincha");
    }
    else if (Data.Contains(TEXT("引导秘籍怎么触发")) ||
             Data.Contains(TEXT("领导秘籍怎么处理")) ||
             Data.Contains(TEXT("领导秘籍怎么处理")) ) 
    {
        strCmbox = TEXT("donggandidai1");
    }
    else if (Data.Contains(TEXT("探秘馆里有什么")) ||
             Data.Contains(TEXT("叛秘管理有什么")) ||
             Data.Contains(TEXT("蜂蜜管里有什么")))
    {
        strCmbox = TEXT("donggandidai2"); 
    }
    else if (Data.Contains(TEXT("在馆内怎么使用")) ||
             Data.Contains(TEXT("传承令在国内"))   ||   
             Data.Contains(TEXT("在管内怎么使用"))   ||  
             Data.Contains(TEXT("秦小黑探秘管理"))   ||   
             Data.Contains(TEXT("探秘管理有什么"))   ||   
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("传承令在国内"))   ||   //====== 预留选项 =======
             Data.Contains(TEXT("在管内怎么使用")))
    {
        strCmbox = TEXT("meihaoshiguanghaitai");
    }
    // 你可以继续添加更多的条件
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Data does not contain any specified strings"));
        // 如果不是预设的语音则设置管道恢复接收数据
        StartAcceptingData();

        return;
    }

    // 在主线程上执行 UI 修改
    if (InteractiveIns && InteractiveIns->strCmbAni)
    {
        AsyncTask(ENamedThreads::GameThread, [InteractiveIns = this->InteractiveIns, strCmbox]()
        {
            InteractiveIns->strCmbAni->SetSelectedOption(strCmbox);
            InteractiveIns->MyUpdateImages(0);
        });
    }
}
