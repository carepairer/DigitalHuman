// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/VideoSoundActor.h"
#include <playlist.h>

AVideoSoundActor::AVideoSoundActor()
{
    // 启用 Tick 功能
    PrimaryActorTick.bCanEverTick = true;
    // 创建并初始化 MediaSoundComponent
    MediaSoundComponent = CreateDefaultSubobject<UMediaSoundComponent>(TEXT("MediaSoundComponent"));

    // 将 MediaSoundComponent 设置为根组件
    RootComponent = MediaSoundComponent;

    // 创建并初始化 MediaPlayer
    MediaPlayer = CreateDefaultSubobject<UMediaPlayer>(TEXT("MediaPlayer"));

    // 创建并初始化 MediaTexture
    MediaTexture = CreateDefaultSubobject<UMediaTexture>(TEXT("MediaTexture"));

}

void AVideoSoundActor::BeginPlay()
{
    Super::BeginPlay();

    if (MediaPlayer)
    {
        MediaSoundComponent->SetMediaPlayer(MediaPlayer);
    }
}


