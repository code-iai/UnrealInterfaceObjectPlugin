// Fill out your copyright notice in the Description page of Project Settings.

#include "AObjectPosePublisher.h"
#include "Engine/World.h"
#include "UTags/Public/Tags.h"
#include "Map.h"
#include "Conversions.h"

// Sets default values
AObjectPosePublisher::AObjectPosePublisher()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AObjectPosePublisher::BeginPlay()
{
	Super::BeginPlay();

	// Checking for nullptr
	check(GetGameInstance());

	UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());
	check(ActiveGameInstance);

	check(ActiveGameInstance->ROSHandler.IsValid());

	// PoseStamped Publisher
	Publisher = MakeShareable<FROSBridgePublisher>(new FROSBridgePublisher(PublisherTopic, TEXT("geometry_msgs/PoseStamped")));
	ActiveGameInstance->ROSHandler->AddPublisher(Publisher);
	
	ActiveGameInstance->ROSHandler->Process();
}

// Called every frame
void AObjectPosePublisher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//TSharedPtr<std_msgs::String> StringMsgPtr(new std_msgs::String(TEXT("TEST")));
	TSharedPtr<geometry_msgs::PoseStamped> PoseMsgPtr(new geometry_msgs::PoseStamped(std_msgs::Header(12,FROSTime::Now(),TEXT("Test")), geometry_msgs::Pose(geometry_msgs::Point(1,1,1), geometry_msgs::Quaternion())));

	// Current ROS Game instance.
	UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());
	check(ActiveGameInstance);
	// 1 Sekunde belassen fürs erst.
	TimeCounterRosCall += DeltaTime;
	if (TimeCounterRosCall >= 1.00)
	{
		TimeCounterRosCall -= 1.00;
		PublishAllObjectsWithTag(ActiveGameInstance, PublisherTopic, TypetoPublish);
	}
	ActiveGameInstance->ROSHandler->Process();
}

void AObjectPosePublisher::PublishAllObjectsWithTag(UROSBridgeGameInstance* Instance, FString Topic, FString TypeTag)
{
	// Set up a Handler Variable and an Actor Map.
	TSharedPtr<FROSBridgeHandler> Handler = Instance->ROSHandler;
	TMap<AActor*, FJsonSerializableKeyValueMap> ActorMap = FTags::GetActorsToKeyValuePairs(this->GetWorld(), TypeTag);

	TArray<AActor*> KeyList;
	FJsonSerializableKeyValueMap TagValues;

	ActorMap.GetKeys(KeyList);

	for (int i = 0; i < ActorMap.Num(); i++)
	{
		TagValues = FTags::GetKeyValuePairs(KeyList[i], FString("SemLog"));
		FString* uId = TagValues.Find("Id");
		if (!uId)
		{
			if (bDebug)
				UE_LOG(LogTemp, Error, TEXT("ERROR: ID not found in SemLog"));

			return;
		}
			
		TSharedPtr<geometry_msgs::PoseStamped> PoseMsgPtr(
			new geometry_msgs::PoseStamped(
				std_msgs::Header(0, FROSTime::Now(), *uId),
				geometry_msgs::Pose(
					geometry_msgs::Point(      FConversions::UToROS(KeyList[i]->GetActorLocation())  ),
					geometry_msgs::Quaternion( FConversions::UToROS(KeyList[i]->GetActorRotation().Quaternion()) )
				)
			)
		);
		Handler->PublishMsg(Topic, PoseMsgPtr);
	}
}
