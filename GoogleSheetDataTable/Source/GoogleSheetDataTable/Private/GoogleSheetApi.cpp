// Copyright Epic Games, Inc. All Rights Reserved.
#include "GoogleSheetApi.h"

const FString UGoogleSheetApi::SheetApiUrl = "https://docs.google.com/spreadsheets/d/";

TSharedRef<IHttpRequest> UGoogleSheetApi::GetRequest(FString suburl)
{
	Http = &FHttpModule::Get();
	TSharedRef<IHttpRequest> Request = Http->CreateRequest();
	Request->SetURL(UGoogleSheetApi::SheetApiUrl + suburl);

	Request->SetHeader(TEXT("User-Agent"), TEXT("X-UnrealEngine-Agent"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("text/csv"));
	Request->SetHeader(TEXT("Accepts"), TEXT("text/csv"));

	Request->SetVerb("GET");
	return Request;
}

void UGoogleSheetApi::SendRequest(FString sheetID)
{
	TSharedRef<IHttpRequest> Request = GetRequest(FString::Printf(TEXT("%s/export?format=csv"), *sheetID));
	Request->OnProcessRequestComplete().BindUObject(this, &UGoogleSheetApi::ProcessResponse);
	Request->ProcessRequest();
}

bool UGoogleSheetApi::ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful || !Response.IsValid())
	{
		return false;
	}

	if (EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		return true;
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("Http error : %d"), Response->GetResponseCode());
		return false;
	}
}

void UGoogleSheetApi::ProcessResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!ResponseIsValid(Response, bWasSuccessful))
	{
		return;
	}

	OnResponseDelegate.ExecuteIfBound(Response->GetContentAsString());
}