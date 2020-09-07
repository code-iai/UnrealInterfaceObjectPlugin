#pragma once

#include "ROSBridgeSrvServer.h"
#include "std_srvs/SetBool.h"

class FROSCallTouchingObjects final : public FROSBridgeSrvServer
{
public:
	bool bSetBoolean = false;

	FROSCallTouchingObjects(const FString InName, FString InType) : FROSBridgeSrvServer(InName, InType)
	{
	}

	TSharedPtr<FROSBridgeSrv::SrvRequest> FromJson(TSharedPtr<FJsonObject> JsonObject) const override
	{
		TSharedPtr<std_srvs::SetBool::Request> Request = MakeShareable(new std_srvs::SetBool::Request());
		Request->FromJson(JsonObject);
		return TSharedPtr<FROSBridgeSrv::SrvRequest>(Request);
	}

	TSharedPtr<FROSBridgeSrv::SrvResponse> Callback(TSharedPtr<FROSBridgeSrv::SrvRequest> InRequest) override
	{
		TSharedPtr<std_srvs::SetBool::Request> Request = StaticCastSharedPtr<std_srvs::SetBool::Request>(InRequest);

		bSetBoolean = Request->GetData();
		UE_LOG(LogTemp, Warning, TEXT("Boolean was set."));
		return MakeShareable<FROSBridgeSrv::SrvResponse>(new std_srvs::SetBool::Response(true, FString("bLinkedBool has been set.")));
	}
};