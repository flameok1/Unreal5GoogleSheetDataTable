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
	UPROPERTY() //�n����flag�~�|�s�Jjson
		/// <summary>
		/// table name map to google sheet id
		/// </summary>
		TMap<FString, FString> googleApiIDMap;
};

class FGoogleSheetDataTableModule : public IModuleInterface
{
private:
	/// <summary>
	/// �T�w�x�sgoogle api setting path
	/// </summary>
	static const FString FILE_PATH;

	/// <summary>
	/// �[��menu �� delegate
	/// </summary>
	FDelegateHandle _cbExtenderDelegateHandle;
	
	/// <summary>
	/// �ثe��ܪ�data table
	/// </summary>
	UDataTable* _selectedDataTable;

	/// <summary>
	/// �Ȧs sheet id
	/// </summary>
	FString _cacheSheetId;

	/// <summary>
	/// �]�w������ �Ȧs sheet id
	/// </summary>
	FString _cacheSetSheetId;

	/// <summary>
	/// ����ptr
	/// </summary>
	TSharedPtr<SWindow> _windowPtr;

	/// <summary>
	/// google api obj
	/// </summary>
	UGoogleSheetApi* _googleSheetApi;

	/// <summary>
	/// ��������
	/// </summary>
	void CloseWindow();
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/// <summary>
	/// �ˬdgoogle api setting���
	/// </summary>
	void CheckGoogleApiSetting();

	/// <summary>
	/// ���osheet id from google api setting���
	/// </summary>
	bool GetSheetIDFromFile(FString& output, const FString& path, const FString& tablename);

	/// <summary>
	/// ��ssheet id to google api setting���
	/// </summary>
	void UpdateSheetIDToFile(FString& sheetid, const FString& path, const FString& tablename);

	/// <summary>
	/// http api Response
	/// </summary>
	void OnApiResponse(FString Response);

	/// <summary>
	/// ContentBrowser ���[ menu �|�I�s
	/// </summary>
	TSharedRef<FExtender> DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList);

	/// <summary>
	/// �[�J menu
	/// </summary>
	void AddMenuEntry(FMenuBuilder& MenuBuilder);

	/// <summary>
	/// ���window
	/// </summary>
	void ShowWindow();

	/// <summary>
	/// ��ܳ]�wwindow
	/// </summary>
	void ShowSetWindow();

	/// <summary>
	/// ��ssheet id
	/// </summary>
	void ChangeSheetID(const FText& InValue);

	/// <summary>
	/// ��sset sheet id
	/// </summary>
	void ChangeSetSheetID(const FText& InValue);

	/// <summary>
	/// �H�U�Obutton �B�z
	/// </summary>
	FReply OnOKClicked();
	FReply OnSaveClicked();
	FReply OnCancelClicked();

	/// <summary>
	/// �I�sgoogle api��function
	/// </summary>
	void UpdateDataTableFromGoogleSheet();
};
