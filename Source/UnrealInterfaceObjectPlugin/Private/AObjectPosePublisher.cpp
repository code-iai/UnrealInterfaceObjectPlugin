// Fill out your copyright notice in the Description page of Project Settings.

#include "AObjectPosePublisher.h"
#include "Engine/World.h"
#include "UTags/Public/Tags.h"
#if ENGINE_MINOR_VERSION < 25 && ENGINE_MAJOR_VERSION == 4
#include "Map.h"
#elif ENGINE_MAJOR_VERSION >= 5
#include "Containers/Map.h"
#endif
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

	Publisher = MakeShareable< FROSBridgePublisher>(new FROSBridgePublisher(PublisherTopic, TEXT("tf2_msgs/TFMessage")));
	ActiveGameInstance->ROSHandler->AddPublisher(Publisher);

	ActiveGameInstance->ROSHandler->Process();
}

// Called every frame
void AObjectPosePublisher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Current ROS Game instance.
	UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());
	check(ActiveGameInstance);
	// 1 Sekunde belassen f�rs erst.
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

	TArray<geometry_msgs::TransformStamped> OutputArray;

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

		std_msgs::Header Header = std_msgs::Header(i, FROSTime::Now(), *uId);
		geometry_msgs::Transform Transform = geometry_msgs::Transform(KeyList[i]->GetActorLocation(), KeyList[i]->GetActorRotation().Quaternion());

		OutputArray.Add(geometry_msgs::TransformStamped(Header, "Nothing", Transform));
	}


	TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr(
		new tf2_msgs::TFMessage(OutputArray)
	);

	Handler->PublishMsg(Topic, TFMsgPtr);
}
