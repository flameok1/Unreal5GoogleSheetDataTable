// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "GoogleSheetApi.h"
#include "GoogleSheetDataTable.generated.h"




USTRUCT()
struct FGoogleApiSetting
{
	GENERATED_BODY()
public:
	UPROPERTY() //要有此flag才會存入json
		/// <summary>
		/// table name map to google sheet id
		/// </summary>
		TMap<FString, FString> googleApiIDMap;
};

class FGoogleSheetDataTableModule : public IModuleInterface
{
private:
	/// <summary>
	/// 固定儲存google api setting path
	/// </summary>
	static const FString FILE_PATH;

	/// <summary>
	/// 加到menu 的 delegate
	/// </summary>
	FDelegateHandle _cbExtenderDelegateHandle;
	
	/// <summary>
	/// 目前選擇的data table
	/// </summary>
	UDataTable* _selectedDataTable;

	/// <summary>
	/// 暫存 sheet id
	/// </summary>
	FString _cacheSheetId;

	/// <summary>
	/// 設定視窗用 暫存 sheet id
	/// </summary>
	FString _cacheSetSheetId;

	/// <summary>
	/// 視窗ptr
	/// </summary>
	TSharedPtr<SWindow> _windowPtr;

	/// <summary>
	/// google api obj
	/// </summary>
	UGoogleSheetApi* _googleSheetApi;

	/// <summary>
	/// 關閉視窗
	/// </summary>
	void CloseWindow();
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// <summary>
	/// 檢查google api setting資料
	/// </summary>
	void CheckGoogleApiSetting();

	/// <summary>
	/// 取得sheet id from google api setting資料
	/// </summary>
	bool GetSheetIDFromFile(FString& output, const FString& path, const FString& tablename);

	/// <summary>
	/// 更新sheet id to google api setting資料
	/// </summary>
	void UpdateSheetIDToFile(FString& sheetid, const FString& path, const FString& tablename);

	/// <summary>
	/// http api Response
	/// </summary>
	void OnApiResponse(FString Response);

	/// <summary>
	/// ContentBrowser 附加 menu 會呼叫
	/// </summary>
	TSharedRef<FExtender> DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList);

	/// <summary>
	/// 加入 menu
	/// </summary>
	void AddMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// 顯示window
	/// </summary>
	void ShowWindow();

	/// <summary>
	/// 顯示設定window
	/// </summary>
	void ShowSetWindow();

	/// <summary>
	/// 更新sheet id
	/// </summary>
	void ChangeSheetID(const FText& InValue);

	/// <summary>
	/// 更新set sheet id
	/// </summary>
	void ChangeSetSheetID(const FText& InValue);

	/// <summary>
	/// 以下是button 處理
	/// </summary>
	FReply OnOKClicked();
	FReply OnSaveClicked();
	FReply OnCancelClicked();

	/// <summary>
	/// 呼叫google api用function
	/// </summary>
	void UpdateDataTableFromGoogleSheet();
};
