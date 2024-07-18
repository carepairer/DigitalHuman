// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Templates/SharedPointer.h" 

#include "Interactive.generated.h"
 


class UMediaPlayer;
class UMediaTexture;
class UImage;
class UButton;
class UComboBoxString;
class AMyPlayerController;
class NamedPipeSerRunnable;
class UMyMediaPlayer;
class UFileMediaSource;
class USoundWave;
class AVideoSoundActor;

/**
 * 
 */
UCLASS()
class DIGITALHUMAN_API UInteractive : public UUserWidget
{
	GENERATED_BODY()

public:
    UInteractive(const FObjectInitializer& ObjectInitializer); // 带参构造函数,类"UUserwidget"不存在默认构造函数

    virtual void NativeOnInitialized() override;
    virtual void BeginDestroy() override;

    // 用于在编辑器中绑定控件
    virtual void NativeConstruct() override;

    // 按钮点击事件的处理函数
    UFUNCTION()
    void ShowNext();
    UFUNCTION()
    void ShowPrev();




    UFUNCTION()
    void OnVideoEnd();

    // 更新显示的图片
    UFUNCTION(BlueprintCallable, Category = "ImageSwitcher")
    void MyUpdateImages(int32 nCurIdx);

    UFUNCTION()
    void LoadImagesFromDisk(const FString& FolderPath);

    UFUNCTION()
    void OnMediaOpened(FString OpenedUrl);
    UFUNCTION()
    void OnMediaOpenFailed(FString FailedUrl);


    UFUNCTION()
    void OnMediaEndReached();


    UPROPERTY()
    int32 nCurrImgTexIdx;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
    UMediaPlayer* MediaPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
    UMediaTexture* MediaTexture;

    UPROPERTY()
    TObjectPtr<UFileMediaSource> MediaSource;

    UPROPERTY()
    TObjectPtr<USoundWave> SoundWave;

    UPROPERTY()
    FString VideoPath;

    UPROPERTY()
    FString AudioFilePath;

    UFUNCTION()
    bool InitializeMediaComponents();

    UFUNCTION()
    bool LoadAudioFromFile(const FString& FilePath);


    //用来播图片或者视频
    UPROPERTY(meta = (BindWidget, BlueprintReadWrite))
    TObjectPtr<UImage> imgDispl;

    //图片纹理数组
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "arrImage")
    TArray<UTexture2D*> arrImgTex;

    FTimerHandle FlipBookTimerHandle;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> btnPrev;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> btnNext;




    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UComboBoxString> strCmbAni;

    UPROPERTY()
    TObjectPtr<AMyPlayerController> myCtrl;

    TSharedPtr<NamedPipeSerRunnable> NamedPipeServerRunnable;
    FRunnableThread* NamedPipeServerThread;
};
