// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MediaSoundComponent.h"
#include "MediaPlayer.h"
#include "MediaTexture.h" 
#include "Engine/Texture2D.h"
#include "VideoSoundActor.generated.h"

UCLASS()
class DIGITALHUMAN_API AVideoSoundActor : public AActor
{
    GENERATED_BODY()

public:
    AVideoSoundActor();



protected:
    virtual void BeginPlay() override;


public:

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
    UMediaSoundComponent* MediaSoundComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media")
    UMediaPlayer* MediaPlayer;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Media") 
    UMediaTexture* MediaTexture;
};
