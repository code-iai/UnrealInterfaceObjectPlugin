// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ROSBridgePublisher.h"
#include "ROSBridgeGameInstance.h"
#include "FROSCallTouchingObjects.h"
#include "tf2_msgs/TFMessage.h"
#include "geometry_msgs/TransformStamped.h"
#include "std_msgs/String.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "AObjectPhysicsPublisher.generated.h"

UCLASS()
class UNREALINTERFACEOBJECTPLUGIN_API AObjectPhysicsPublisher : public AActor
{
	GENERATED_BODY()

public:
	AObjectPhysicsPublisher();
	TSharedPtr<FROSBridgePublisher> Publisher;
	TSharedPtr<FROSBridgePublisher> ProblemPublisher;
	TSharedPtr<FROSBridgePublisher> StatePublisher;
	TSharedPtr<FROSCallTouchingObjects> Service;

	UPROPERTY(EditAnywhere)
	AActor* FocusObject;

	UPROPERTY(EditAnywhere)
	FString ProblemPublisherTopic = FString("/unreal_interface/physics_spawn_problem");
	UPROPERTY(EditAnywhere)
	FString PublisherTopic = FString("/unreal_interface/physics_contact");
	UPROPERTY(EditAnywhere)
	FString StateTopic = FString("/unreal_interface/state_publisher");
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
	// Toggleable additional checks to determine if an object is within, under or up from the object.
	UPROPERTY(EditAnywhere)
	bool bDetermineDirection = false;
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
		Function Creating invisible Collision Boxes around each and every Object bearing the Searched TrackTags
		in the given array by using the BoundingBox of the StaticMesh itself rather than ingame.

		@param ActorArray Array of Actor given
		@param TrackTag A Tag for which is searched and used to decide on which Object it'll put the box over.
		@return Reference to the created Collision Box
	*/
	UBoxComponent* CreateBoxChecker(AActor* ActorA);

	/*
		Function to determine whether the given object fall under the Category of Up, 
		Down or Inside the object with the help of vectors.
		@param OriginObject Input with the Object which should be the reference for the determination.
		@param ObjectToCheckFor Input is the Actor which touches the object for which to check it for.
		@return A Fstring, for now, which simply states back whether the given Object is inside, ontop or under.
	*/
	FString DetermineState(AActor* OriginObject, AActor* ObjectToCheckFor);

	/*
		Auxiliary Function to help determine the state of another Object B relative to Object A.

		@param A FromActor from where the Vecotr should begin with.
		@param B ToActor where the Vector should go to.
		@return FVector with the Vector going from A to B.
	*/
	FVector GetVectorFromAToB(AActor* A, AActor* B);
};