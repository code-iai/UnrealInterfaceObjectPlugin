#include "AObjectPhysicsPublisher.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "Map.h"
#include "UTags/Public/Tags.h"

AObjectPhysicsPublisher::AObjectPhysicsPublisher()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AObjectPhysicsPublisher::BeginPlay()
{
	Super::BeginPlay();

	// Checking for nullptr
	check(GetGameInstance());
	UROSBridgeGameInstance* ActiveGameInstance = Cast<UROSBridgeGameInstance>(GetGameInstance());

	check(ActiveGameInstance->ROSHandler.IsValid());
	
	Service = MakeShareable<FROSCallTouchingObjects>(new FROSCallTouchingObjects(ServiceTopic, TEXT("std_srvs/SetBool")));
	Publisher = MakeShareable<FROSBridgePublisher>(new FROSBridgePublisher(PublisherTopic, TEXT("tf2_msgs/TFMessage")));
	ProblemPublisher = MakeShareable<FROSBridgePublisher>(new FROSBridgePublisher(ProblemPublisherTopic, TEXT("tf2_msgs/TFMessage")));
	ActiveGameInstance->ROSHandler->AddPublisher(Publisher);
	ActiveGameInstance->ROSHandler->AddPublisher(ProblemPublisher);
	ActiveGameInstance->ROSHandler->AddServiceServer(Service);
	ActiveGameInstance->ROSHandler->Process();
}

void AObjectPhysicsPublisher::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Current RosGameInstance
	UROSBridgeGameInstance* ActiveGameInstace = Cast<UROSBridgeGameInstance>(GetGameInstance());
	check(ActiveGameInstace);

	// Counter Work
	ROSCallIntervall += DeltaTime;
	if (ROSCallIntervall >= 0.50)
	{
		ROSCallIntervall -= 0.50;

		bGiveAllTrackedTouches = Service->bSetBoolean;
		PublishCollidingObject(ActiveGameInstace);
	}

	ActiveGameInstace->ROSHandler->Process();
}

void AObjectPhysicsPublisher::PublishCollidingObject(UROSBridgeGameInstance* Instance)
{
	TSharedPtr<FROSBridgeHandler> Handler = Instance->ROSHandler;
	TArray<AActor*> ActorList = GetTaggedActors(TypeToTrack);
	TArray<geometry_msgs::TransformStamped> OutputArray;
	
	for (int i = 0; i < ActorList.Num(); i++)
	{
		// Check State To Focus Object
		if (bDetermineDirection && FocusObject)
			DetermineState(FocusObject, ActorList[i]);
		
		FJsonSerializableKeyValueMap Tags = FTags::GetKeyValuePairs(ActorList[i], FString("SemLog"));
		FString* uId = Tags.Find("Id");
		if (!uId)
		{
			UE_LOG(LogTemp, Error, TEXT("ERROR: ID not found in SemLog"));
			return;
		}

		if (ActorList[i]->GetComponentsByClass(UBoxComponent::StaticClass()).Num() == 0)
		{
			UBoxComponent* compBox = CreateBoxChecker(ActorList[i]);

			if (bDebug)
				UE_LOG(LogTemp, Log, TEXT("Begin checking for spawn overlap collisions"));

			TArray<geometry_msgs::TransformStamped> Checked = CheckSpawnCollision(compBox);

			if (Checked.Num() != 0)
				ActorList[i]->Destroy();

			TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr(
				new tf2_msgs::TFMessage(Checked)
			);

			Handler->PublishMsg(ProblemPublisherTopic, TFMsgPtr);
		}
		else if (bGiveAllTrackedTouches)
		{
			TArray<UPrimitiveComponent*> OutArrayComponents;
			Cast<UBoxComponent>(ActorList[i]->GetComponentsByClass(UBoxComponent::StaticClass())[0])->GetOverlappingComponents(OutArrayComponents);

			for (UPrimitiveComponent * Comp : OutArrayComponents)
			{
				OutputArray.Add(geometry_msgs::TransformStamped(std_msgs::Header(i, FROSTime::Now(), *uId), 
					Comp->GetAttachmentRootActor()->GetName(), 
					geometry_msgs::Transform(Comp->GetComponentLocation(), 
					Comp->GetComponentQuat())));

				if (bDebug)
					UE_LOG(LogTemp, Warning, TEXT("%s touches %s"), *ActorList[i]->GetName(), *Comp->GetAttachmentRootActor()->GetName());
			}

			if (bDebug)
			{
				FVector Extent = Cast<UStaticMeshComponent>(ActorList[i]->GetRootComponent())->GetStaticMesh()->GetBoundingBox().GetExtent() + FVector(1, 1, 1);
				DrawDebugBox(GetWorld(), ActorList[i]->GetActorLocation(), Extent, FColor::Blue, false, 0.5f);
			}

			TSharedPtr<tf2_msgs::TFMessage> TFMsgPtr(
				new tf2_msgs::TFMessage(OutputArray)
			);

			Handler->PublishMsg(PublisherTopic, TFMsgPtr);
		}
	}

	Service->bSetBoolean = false;
	bGiveAllTrackedTouches = false;
}

TArray<AActor*> AObjectPhysicsPublisher::GetTaggedActors(FString InputTypeTag)
{
	TMap<AActor*, FJsonSerializableKeyValueMap> ActorMap = FTags::GetActorsToKeyValuePairs(this->GetWorld(), InputTypeTag);
	TArray<AActor*> ActorList;
	ActorMap.GetKeys(ActorList);

	return ActorList;
}

TArray<geometry_msgs::TransformStamped> AObjectPhysicsPublisher::CheckSpawnCollision(UBoxComponent * Comp)
{
	TArray<geometry_msgs::TransformStamped> Output;
	FString ActorName = Comp->GetOwner()->GetName();
	TArray<UPrimitiveComponent*> OverlappingComponents;

	// Update Overlap Information before the next call.
	Comp->UpdateOverlaps();
	Comp->GetOverlappingComponents(OverlappingComponents);

	if (OverlappingComponents.Num() > 0)
	{
		UE_LOG(LogTemp, Log, TEXT("Spawn Collision Problem detected for %s"), *ActorName);
		for (UPrimitiveComponent* Comp : OverlappingComponents)
		{
			Output.Add(geometry_msgs::TransformStamped(std_msgs::Header(0, FROSTime::Now(), ActorName), 
				Comp->GetAttachmentRootActor()->GetName(), 
				geometry_msgs::Transform(Comp->GetComponentLocation(), Comp->GetComponentQuat())));
		}
	}
	return Output;
}

UBoxComponent* AObjectPhysicsPublisher::CreateBoxChecker(AActor* Actor)
{
	FVector Extent = Cast<UStaticMeshComponent>(Actor->GetRootComponent())->GetStaticMesh()->GetBoundingBox().GetExtent() + FVector(1, 1, 1);

	UBoxComponent* Box;
	Box = NewObject<UBoxComponent>(Actor->GetRootComponent(), FName("CollisionAmountChecker"));
	Box->AttachToComponent(Actor->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
	Box->SetBoxExtent(Extent);
	Box->SetCollisionProfileName(FName("OverlapAll"));
	Box->SetGenerateOverlapEvents(true);
	Box->RegisterComponent();

	if (bDebug)
		UE_LOG(LogTemp, Warning, TEXT("DEBUG: OverlapBox Created over %s"), *Box->GetAttachmentRootActor()->GetName());
	
	return Box;
}

FString AObjectPhysicsPublisher::DetermineState(AActor * OriginObject, AActor * ObjectToCheckFor)
{
	if (OriginObject == ObjectToCheckFor)
		return FString("SELF");

	FString Output = FString();
	// FVector VectorToObject = GetVectorFromAToB(OriginObject, ObjectToCheckFor); // Doesn't work
	FVector VectorToObject = OriginObject->GetActorLocation() - ObjectToCheckFor->GetActorLocation();
	float ActorBoxSizeHeightZ = OriginObject->GetComponentsBoundingBox().GetExtent().Z;
	float ActorBoxSizeHeightY = OriginObject->GetComponentsBoundingBox().GetExtent().Y;
	float ActorBoxSizeHeightX = OriginObject->GetComponentsBoundingBox().GetExtent().X;

	if (bDebug)
	{
		DrawDebugDirectionalArrow(GetWorld(), OriginObject->GetActorLocation(), VectorToObject, 2.0f, FColor::Blue,
			false, 1.0f);
		//UE_LOG(LogTemp, Error, TEXT("%f , %f , %f"), VectorToObject.X, VectorToObject.Y, VectorToObject.Z)
	}

	// Many cases are possible at once, but priotiy is from weakest to strongest
	// Inside, On = Under, Above = Below
	// WIP: Alternative Solution for IN
	bool bInside = VectorToObject.Z <= ActorBoxSizeHeightZ && VectorToObject.Z >= -ActorBoxSizeHeightZ
		&& VectorToObject.Y <= ActorBoxSizeHeightY && VectorToObject.Y >= -ActorBoxSizeHeightY
		&& VectorToObject.X <= ActorBoxSizeHeightX && VectorToObject.X >= -ActorBoxSizeHeightX;
	bool bOn = false; // WIP: TODO Later
	bool bUnder = false; // WIP: TODO Later
	bool bAbove = VectorToObject.Z >= 0;
	bool bBelow = VectorToObject.Z < 0;

	// Start Building Output
	if (bInside && Output.IsEmpty())
		Output = FString("INSIDE");
	if (bOn && Output.IsEmpty())
		Output = FString("ON");
	if (bUnder && Output.IsEmpty())
		Output = FString("UNDER");
	if (bAbove && Output.IsEmpty())
		Output = FString("ABOVE");
	if (bBelow && Output.IsEmpty())
		Output = FString("BELOW");

	if (bDebug && (Output != FString("SELF")))
		UE_LOG(LogTemp, Warning, TEXT("%s is %s of %s"), *ObjectToCheckFor->GetName(), *Output, *OriginObject->GetName());

	return Output;
}

FVector AObjectPhysicsPublisher::GetVectorFromAToB(AActor * A, AActor * B)
{
	FVector Output = UKismetMathLibrary::GetDirectionUnitVector(A->GetActorLocation(), B->GetActorLocation());

	return Output;
}
