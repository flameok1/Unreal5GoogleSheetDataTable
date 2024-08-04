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
	/// cb 資料
	/// </summary>
	OnResponse OnResponseDelegate;

	/// <summary>
	/// 送出http request
	/// </summary>
	void SendRequest(FString sheetID);

private:
	/// <summary>
	/// google api 固定 url
	/// </summary>
	static const FString SheetApiUrl;

	/// <summary>
	/// http obj
	/// </summary>
	FHttpModule* Http;

	/// <summary>
	/// 取得 get 用 req
	/// </summary>
	TSharedRef<IHttpRequest> GetRequest(FString suburl);

	/// <summary>
	/// 處理 http 回應
	/// </summary>
	void ProcessResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	/// <summary>
	/// 檢查http 回應是否有效
	/// </summary>
	bool ResponseIsValid(FHttpResponsePtr Response, bool bWasSuccessful);
};
