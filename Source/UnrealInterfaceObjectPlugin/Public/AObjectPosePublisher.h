// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROSBridgePublisher.h"
#include "ROSBridgeGameInstance.h"
#include "tf2_msgs/TFMessage.h"
#include "geometry_msgs/TransformStamped.h"
#if ENGINE_MINOR_VERSION < 25 || ENGINE_MAJOR_VERSION >4
#include "Array.h"
#endif
#include "AObjectPosePublisher.generated.h"

UCLASS()
class UNREALINTERFACEOBJECTPLUGIN_API AObjectPosePublisher : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AObjectPosePublisher();
	TSharedPtr<FROSBridgePublisher> Publisher;

	// Publisher: Position Tracker
	UPROPERTY(EditAnywhere)
	FString PublisherTopic = FString("/unreal_interface/object_poses");
	UPROPERTY(EditAnywhere)
	FString TypetoPublish = FString("UnrealInterface");
	UPROPERTY(EditAnywhere)
	FString KeyToPublish = FString("spawned");

	UPROPERTY(EditAnywhere)
	bool bDebug = false;

private:
	// Time Counter
	float TimeCounterRosCall = 0.00;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Publishes all Object with the given Tag under a topic to rosbridge.
	void PublishAllObjectsWithTag(UROSBridgeGameInstance* Instance, FString Topic, FString Tag);
};
