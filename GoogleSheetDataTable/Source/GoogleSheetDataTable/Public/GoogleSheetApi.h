#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "GoogleSheetApi.generated.h"

DECLARE_DELEGATE_OneParam(OnResponse, FString);

UCLASS()
class GOOGLESHEETDATATABLE_API UGoogleSheetApi : public UObject
{
	GENERATED_BODY()

public:	
	/// <summary>
	/// cb ���
	/// </summary>
	OnResponse OnResponseDelegate;

	/// <summary>
	/// �e�Xhttp request
	/// </summary>
	void SendRequest(FString sheetID);

private:
	/// <summary>
	/// google api �T�w url
	/// </summary>
	static const FString SheetApiUrl;

	/// <summary>
	/// http obj
	/// </summary>
	FHttpModule* Http;

	/// <summary>
	/// ���o get �� req
	/// </summary>
	TSharedRef<IHttpRequest> GetRequest(FString suburl);

	/// <summary>
	/// �B�z http �^��
	/// </summary>
	void ProcessResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/// <summary>
	/// �ˬdhttp �^���O�_����
	/// </summary>
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);
};
