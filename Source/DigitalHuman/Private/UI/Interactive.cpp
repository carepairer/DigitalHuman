// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Interactive.h"
#include "MediaPlayer.h"
#include "MediaTexture.h"
#include "Components/Image.h"
#include "TimerManager.h"
#include "Engine/Texture2D.h"
#include "Components/Button.h"
#include "Util/MyUtil.h"
#include "UObject/ConstructorHelpers.h"
#include <FileMediaSource.h>
#include <Components/ComboBoxString.h>
#include "Ctrl/MyPlayerController.h"
#include "Pipe/NamedPipeSerRunnable.h"
#include <Kismet/GameplayStatics.h>


#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFilemanager.h"
#include "AudioDecompress.h"
#include "Interfaces/IAudioFormat.h"


#include "Actor/VideoSoundActor.h"
#include <Net/ReplayPlaylistTracker.h>


UInteractive::UInteractive(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{

}

void UInteractive::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    if (btnPrev != nullptr && btnNext != nullptr) {
        btnPrev->OnClicked.AddDynamic(this, &UInteractive::ShowPrev);
        btnNext->OnClicked.AddDynamic(this, &UInteractive::ShowNext);
    }

    nCurrImgTexIdx = 0;


    // 获取并转换PlayerController
    APlayerController* BaseController = GetOwningPlayer();
    if (BaseController)
    {
        myCtrl = Cast<AMyPlayerController>(BaseController);
    }


    // 创建并启动 NamedPipeServerRunnable 线程
    NamedPipeServerRunnable = MakeShareable(new NamedPipeSerRunnable(this));
    NamedPipeServerThread = FRunnableThread::Create(NamedPipeServerRunnable.Get(), TEXT("NamedPipeServerThread"));

    // 检查 NamedPipeServerThread 是否成功创建
    if (!NamedPipeServerThread)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create NamedPipeServerThread"));
    }
}


void UInteractive::BeginDestroy()
{
    Super::BeginDestroy();

    // 停止并清理 NamedPipeServerRunnable
    if (NamedPipeServerThread)
    {
        NamedPipeServerThread->Kill(true);
        delete NamedPipeServerThread;
        NamedPipeServerThread = nullptr;
    }

    NamedPipeServerRunnable.Reset();
}

void UInteractive::NativeConstruct()
{
    Super::NativeConstruct();

    // 检查 imgDispl 是否已正确绑定
    if (!imgDispl)
    {
        UE_LOG(LogTemp, Warning, TEXT("imgDispl is not bound"));
        return;
    }
}

bool UInteractive::InitializeMediaComponents()
{
    // 检查并创建 MediaPlayer
    if (!MediaPlayer)
    {
        MediaPlayer = NewObject<UMediaPlayer>(this, UMediaPlayer::StaticClass());
        if (!MediaPlayer)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create MediaPlayer"));
            return false;
        }

        // 在初始化媒体组件后，设置播放结束的委托
        MediaPlayer->OnEndReached.AddDynamic(this, &UInteractive::OnMediaEndReached);
    }

    // 检查并创建 MediaTexture
    if (!MediaTexture)
    {
        MediaTexture = NewObject<UMediaTexture>(this, UMediaTexture::StaticClass());
        if (MediaTexture)
        {
            MediaTexture->SetMediaPlayer(MediaPlayer);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create MediaTexture"));
            return false;
        }
    }

    // 检查并创建 MediaSource
    if (!MediaSource)
    {
        MediaSource = NewObject<UFileMediaSource>(GetTransientPackage(), NAME_None, RF_Transactional | RF_Transient);
        if (!MediaSource)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create MediaSource"));
            return false;
        }
    }

    return true;
}

bool UInteractive::LoadAudioFromFile(const FString& FilePath)
{
    // 加载文件数据
    TArray<uint8> RawFileData;
    if (!FFileHelper::LoadFileToArray(RawFileData, *FilePath))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load file: %s"), *FilePath);
        return false;
    }

    // 如果 SoundWave 未初始化，则创建一个新的 SoundWave 对象
    if (!SoundWave)
    {
        SoundWave = NewObject<USoundWave>(this, USoundWave::StaticClass());
        if (!SoundWave)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to create SoundWave object"));
            return false;
        }
    }

    // 解析音频文件信息
    FWaveModInfo WaveInfo;
    if (!WaveInfo.ReadWaveInfo(RawFileData.GetData(), RawFileData.Num()))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse wave file: %s"), *FilePath);
        return false;
    }

    // 释放之前分配的内存（如果有）
    if (SoundWave->RawPCMData)
    {
        FMemory::Free(SoundWave->RawPCMData);
        SoundWave->RawPCMData = nullptr;
    }

    // 设置 SoundWave 的属性
    SoundWave->InvalidateCompressedData();

    int32 Channels = *WaveInfo.pChannels;
    int32 SamplesPerSec = *WaveInfo.pSamplesPerSec;
    int32 SampleDataSize = WaveInfo.SampleDataSize;

    UE_LOG(LogTemp, Log, TEXT("Audio file info - Channels: %d, Sample Rate: %d, Sample Data Size: %d"), Channels, SamplesPerSec, SampleDataSize);

    SoundWave->Duration = static_cast<float>(SampleDataSize) / (2.0f * static_cast<float>(Channels) * static_cast<float>(SamplesPerSec));
    SoundWave->SetSampleRate(SamplesPerSec);
    SoundWave->NumChannels = Channels;

    SoundWave->SoundGroup = SOUNDGROUP_Default;
    SoundWave->RawPCMDataSize = SampleDataSize;
    SoundWave->RawPCMData = static_cast<uint8*>(FMemory::Malloc(SampleDataSize)); // 分配新的堆内存
    FMemory::Memcpy(SoundWave->RawPCMData, WaveInfo.SampleDataStart, SampleDataSize); // 将数据复制到分配的内存中

    // 设置一些额外的参数来确保声音能够正确播放
    SoundWave->bLooping = false;
    SoundWave->bProcedural = false;
    SoundWave->bSingleLine = true;

    return true;
}


void UInteractive::ShowPrev()
{
    MyUpdateImages(0);
    
}


void UInteractive::ShowNext()
{ 

    // 开始接收管道数据
    if (NamedPipeServerRunnable.IsValid())
    {
        NamedPipeServerRunnable->StartAcceptingData();
        UE_LOG(LogTemp, Log, TEXT("Started accepting data."));
    }
}


void UInteractive::OnVideoEnd()
{
    // 播放图片
    nCurrImgTexIdx = (nCurrImgTexIdx) % arrImgTex.Num();
    if (imgDispl && arrImgTex.IsValidIndex(nCurrImgTexIdx))
    {
        FSlateBrush Brush;
        Brush.SetResourceObject(arrImgTex[nCurrImgTexIdx]);
        imgDispl->SetBrush(Brush);
    }
}

void UInteractive::MyUpdateImages(int32 nCurIdx)
{
    //获取图片图片切换动画
    FString SelOption = strCmbAni->GetSelectedOption();

    if (SelOption == TEXT("oppo"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\0-oppo.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\0-oppo.wav");
    }
    else if (SelOption == TEXT("jizhitangjiang"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\1-jizhitangjiang.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\1-jizhitangjiang.wav");
    }
    else if (SelOption == TEXT("lanjv"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\2-lanjv.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\2-lanjv.wav");
    }
    else if (SelOption == TEXT("wangwang"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\3-wangwang.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\3-wangwang.wav");
    }
    else if (SelOption == TEXT("oupaoguonai"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\4-oupaoguonai.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\4-oupaoguonai.wav");
    }
    else if (SelOption == TEXT("mengniusuansuanru"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\5-mengniusuansuanru.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\5-mengniusuansuanru.wav");
    }
    else if (SelOption == TEXT("wahahalongjincha"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\6-wahahalongjincha.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\6-wahahalongjincha.wav");
    }
    else if (SelOption == TEXT("donggandidai1"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\7-dongandidai1.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\7-dongandidai1.wav");
    }
    else if (SelOption == TEXT("donggandidai2"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\8-donggandidai2.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\8-donggandidai2.wav");
    }
    else if (SelOption == TEXT("meihaoshiguanghaitai"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\9-meihaoshiguanghaitai.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\9-meihaoshiguanghaitai.wav");
    }
    else if (SelOption == TEXT("qqtang"))
    {
        VideoPath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\10-qqtang.mp4");
        AudioFilePath = TEXT("C:\\Users\\lin-IT\\Desktop\\UE5Test\\GuHan\\Video\\10-qqtang.wav");
    }
    // 初始化媒体组件
    if (!InitializeMediaComponents())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to initialize media components"));
        return;
    }

    // 更新视频路径
    MediaSource->SetFilePath(VideoPath);

    // 验证媒体源路径
    if (!MediaSource->Validate())
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to validate file path: %s"), *VideoPath);
        return;
    }

    // 打开媒体源
    bool bOpened = MediaPlayer->OpenSource(MediaSource);
    if (!bOpened)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to open media source: %s"), *VideoPath);
        return;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Successfully opened file: %s"), *VideoPath);
    }

    // 更新 MediaTexture
    MediaTexture->UpdateResource();
    FSlateBrush Brush;
    Brush.SetResourceObject(MediaTexture);
    imgDispl->SetBrush(Brush);
    UE_LOG(LogTemp, Log, TEXT("imgDispl brush set with MediaTexture"));

    // 开始播放视频
    MediaPlayer->Play();

    // 加载音频文件
    bool bAudioLoaded = LoadAudioFromFile(AudioFilePath);
    if (bAudioLoaded)
    {
        UGameplayStatics::PlaySound2D(this, SoundWave);
        UE_LOG(LogTemp, Log, TEXT("SoundWave loaded and played successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load SoundWave"));
    }
}

void UInteractive::OnMediaEndReached()
{
    if (NamedPipeServerRunnable.IsValid())
    {
        //管道恢复接收数据
        NamedPipeServerRunnable->StartAcceptingData();
    }
}




void UInteractive::OnMediaOpened(FString OpenedUrl)
{
   
}

void UInteractive::OnMediaOpenFailed(FString FailedUrl)
{
    UE_LOG(LogTemp, Warning, TEXT("Failed to open media: %s"), *FailedUrl);
}


void UInteractive::LoadImagesFromDisk(const FString& FolderPath)
{
    TArray<FString> ImgExt = { TEXT("png"), TEXT("jpg"), TEXT("jpeg") };
    int32 nImgNum = MyUtil::GetImgNumFromPath(FolderPath, ImgExt);

    TObjectPtr<UTexture2D> Texture = nullptr;

    for (int32 i = 0; i < nImgNum; ++i)
    {
        FString FileNamePattern = FString::Printf(TEXT("%d-"), i);

        // 加载第一个文件夹中的图片
        FString FilePathTemp = FPaths::Combine(FolderPath, FileNamePattern);
        MyUtil::FindMatchingFile(FilePathTemp, FilePathTemp);
        MyUtil::LoadImageToTexture(FilePathTemp, Texture);
        arrImgTex.Add(Texture);
    }
}
