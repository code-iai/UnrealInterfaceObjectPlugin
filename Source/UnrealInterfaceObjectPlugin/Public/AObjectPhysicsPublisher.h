// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROSBridgePublisher.h"
#include "ROSBridgeGameInstance.h"
#include "FROSCallTouchingObjects.h"
#include "tf2_msgs/TFMessage.h"
#include "geometry_msgs/TransformStamped.h"
#include "AObjectPhysicsPublisher.generated.h"

UCLASS()
class UNREALINTERFACEOBJECTPLUGIN_API AObjectPhysicsPublisher : public AActor
{
	GENERATED_BODY()

public:
	AObjectPhysicsPublisher();
	TSharedPtr<FROSBridgePublisher> Publisher;
	TSharedPtr<FROSBridgePublisher> ProblemPublisher;
	TSharedPtr<FROSCallTouchingObjects> Service;

	UPROPERTY(EditAnywhere)
	FString ProblemPublisherTopic = FString("/unreal_interface/physics_spawn_problem");
	UPROPERTY(EditAnywhere)
	FString PublisherTopic = FString("/unreal_interface/physics_contact");
	UPROPERTY(EditAnywhere)
	FString TypeToTrack = FString("UnrealInterface");
	UPROPERTY(EditAnywhere)
	FString KeyToTrack = FString("spawned");

	UPROPERTY(EditAnywhere)
	FString ServiceTopic = FString("/unreal_interface/call_touching");

	// Bool
	// Upon being toggled, will result in the collision check event checks.
	UPROPERTY(EditAnywhere)
	bool bGiveAllTrackedTouches;
	UPROPERTY(EditAnywhere)
	bool bDebug = false;

private:
	// Time Counter -- Needed? Test for now
	float ROSCallIntervall = 0.00;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/*
		Publisher Method which will send messages to the two topics

		@param Instance Is the ROS Instance of the running Unreal Engine Game.
	*/
	void PublishCollidingObject(UROSBridgeGameInstance* Instance);

private:
	/*
		Auxialiary Function serving to get all the Actors of the Input Type.

		@param InputTypeTag String with the Type the Actors should have
		@return An Array of Actors which have the InputTypeTag
	*/
	TArray<AActor*> GetTaggedActors(FString InputTypeTag);

	/*
		Auxialiary Function used to check and send out a message for any kind of Spawn Collisions using Overlap Events.

		@param InComponent The Component for which to check its collision for.
		@return An TransformStamped Array with every Object it currently collides with.
	*/
	TArray<geometry_msgs::TransformStamped> CheckSpawnCollision(UPrimitiveComponent * InComponent);

	/*
		Function Creating invisible Collision Boxes around each and every Object bearing the Searched TrackTags
		in the given array by using the BoundingBox of the StaticMesh itself rather than ingame.

		@param ActorArray Array of Actor given
		@param TrackTag A Tag for which is searched and used to decide on which Object it'll put the box over.
	*/
	void CreateBoxChecker(AActor* ActorA);
};