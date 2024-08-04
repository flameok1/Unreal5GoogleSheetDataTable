// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoogleSheetDataTable.h"
#include "LevelEditor.h"
#include "ContentBrowserModule.h"
#include "ContentBrowserDelegates.h"

#include "Editor/MainFrame/Public/Interfaces/IMainFrameModule.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "JsonObjectConverter.h"
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "Runtime/Engine/Classes/Kismet/DataTableFunctionLibrary.h"

#define LOCTEXT_NAMESPACE "FGoogleSheetDataTableModule"

const FString FGoogleSheetDataTableModule::FILE_PATH = "GoogleApiSetting.txt";

void FGoogleSheetDataTableModule::StartupModule()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& menuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	menuExtenderDelegates.Add(FContentBrowserMenuExtender_SelectedAssets::CreateRaw(this, &FGoogleSheetDataTableModule::DataTableContextMenuExtender));
	_cbExtenderDelegateHandle = menuExtenderDelegates.Last().GetHandle();

}

void FGoogleSheetDataTableModule::ShutdownModule()
{
	if (!IsRunningCommandlet() && !IsRunningGame())
	{
		FContentBrowserModule* ContentBrowserModule = (FContentBrowserModule*)(FModuleManager::Get().GetModule(TEXT("ContentBrowser")));
		if (ContentBrowserModule)
		{
			TArray<FContentBrowserMenuExtender_SelectedAssets>& menuExtenderDelegates = ContentBrowserModule->GetAllAssetViewContextMenuExtenders();
			menuExtenderDelegates.RemoveAll([this](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == _cbExtenderDelegateHandle; });
		}
	}
}

TSharedRef<FExtender> FGoogleSheetDataTableModule::DataTableContextMenuExtender(const TArray<FAssetData>& AssetDataList)
{
	if (AssetDataList.Num() != 1 || AssetDataList[0].AssetClassPath.GetAssetName() != UDataTable::StaticClass()->GetFName())
		return MakeShareable(new FExtender());

	_selectedDataTable = Cast<UDataTable>(AssetDataList[0].GetAsset());

	TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());

	MenuExtender->AddMenuExtension(
		"AssetCode",
		EExtensionHook::Before,
		TSharedPtr<FUICommandList>(),
		FMenuExtensionDelegate::CreateRaw(this, &FGoogleSheetDataTableModule::AddMenuEntry));

	return MenuExtender.ToSharedRef();
}

void FGoogleSheetDataTableModule::AddMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString("Load from Google Sheet"),
		FText::FromString("Load from Google Sheet"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGoogleSheetDataTableModule::ShowWindow))
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString("Set Google Sheet ID"),
		FText::FromString("Set Google Sheet ID"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateRaw(this, &FGoogleSheetDataTableModule::ShowSetWindow))
	);
}

void FGoogleSheetDataTableModule::ShowSetWindow()
{
	CheckGoogleApiSetting();

	FString dataTableName;
	_selectedDataTable->GetName(dataTableName);

	if (!GetSheetIDFromFile(_cacheSheetId, FILE_PATH, dataTableName))
	{
		return;
	}

	TSharedRef<SWindow> Window =
		SNew(SWindow).Title(FText::FromString(TEXT("Set Google Sheet ID")))
		.ClientSize(FVector2D(800, 100))
		.SupportsMaximize(false)
		.SupportsMinimize(false)[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).AutoWidth()
						[
							SNew(STextBlock).Text(FText::FromString(TEXT("  Sheet id : ")))
						]
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SEditableTextBox)
								.MinDesiredWidth(800)
								.Text(FText::FromString(_cacheSheetId))
								.BackgroundColor(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
								.OnTextChanged_Raw(this, &FGoogleSheetDataTableModule::ChangeSetSheetID)
						]
				]
				+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
					SNew(SHorizontalBox)
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Save")))
								.OnClicked_Raw(this, &FGoogleSheetDataTableModule::OnSaveClicked)
						]
						+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
						[
							SNew(SButton)
								.Text(FText::FromString(TEXT("Cancel")))
								.OnClicked_Raw(this, &FGoogleSheetDataTableModule::OnCancelClicked)
						]
				]
		];

	_windowPtr = Window;
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

	if (MainFrameModule.GetParentWindow().IsValid()) {
		FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
	}
	else {
		FSlateApplication::Get().AddWindow(Window);
	}
}

void FGoogleSheetDataTableModule::ShowWindow()
{
	CheckGoogleApiSetting();

	FString dataTableName;
	_selectedDataTable->GetName(dataTableName);

	if (!GetSheetIDFromFile(_cacheSheetId, FILE_PATH, dataTableName))
	{
		return;
	}

	if (_cacheSheetId == "")
	{
		TSharedRef<SWindow> Window =
			SNew(SWindow).Title(FText::FromString(TEXT("Load data from Google Sheet")))
			.ClientSize(FVector2D(800, 100))
			.SupportsMaximize(false)
			.SupportsMinimize(false)[
				SNew(SVerticalBox)
					+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
						SNew(SHorizontalBox)
							+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).AutoWidth()
							[
								SNew(STextBlock).Text(FText::FromString(TEXT("  Sheet id : ")))
							]
							+ SHorizontalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
							[
								SNew(SEditableTextBox)
									.MinDesiredWidth(800)
									.Text(FText::FromString(_cacheSheetId))
									.BackgroundColor(FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)))
									.OnTextChanged_Raw(this, &FGoogleSheetDataTableModule::ChangeSheetID)
							]
					]
					+ SVerticalBox::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)[
						SNew(SButton)
							.Text(FText::FromString(TEXT("Ok")))
							.OnClicked_Raw(this, &FGoogleSheetDataTableModule::OnOKClicked)
					]
			];

		_windowPtr = Window;
		IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));

		if (MainFrameModule.GetParentWindow().IsValid()) {
			FSlateApplication::Get().AddWindowAsNativeChild(Window, MainFrameModule.GetParentWindow().ToSharedRef());
		}
		else {
			FSlateApplication::Get().AddWindow(Window);
		}
	}
	else
	{
		UpdateDataTableFromGoogleSheet();
	}
}

void FGoogleSheetDataTableModule::CheckGoogleApiSetting()
{
	FString contentDir = FPaths::ProjectContentDir();

	if (!FPlatformFileManager::Get().GetPlatformFile().FileExists(*(contentDir + FILE_PATH)))
	{
		//沒setting檔案 建置一個空的
		FGoogleApiSetting blankApiSetting;
		FString jsonString;
		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(blankApiSetting);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&jsonString, 0));

		FFileHelper::SaveStringToFile(jsonString, *(contentDir + FILE_PATH));
	}
}

bool FGoogleSheetDataTableModule::GetSheetIDFromFile(FString& output, const FString& path, const FString& tablename)
{
	FString contentDir = FPaths::ProjectContentDir();
	FString fileData;

	output = "";

	if (FFileHelper::LoadFileToString(fileData, *(contentDir + path)))
	{
		FGoogleApiSetting ApiSetting;
		TSharedPtr<FJsonObject> googlesetting;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(fileData), googlesetting);
		if (!FJsonObjectConverter::JsonObjectToUStruct<FGoogleApiSetting>(googlesetting.ToSharedRef(), &ApiSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Can not open GoogleApiSetting"));
			return false;
		}

		if (ApiSetting.googleApiIDMap.Contains(tablename))
		{
			output = ApiSetting.googleApiIDMap[tablename];
		}
		else
		{
			ApiSetting.googleApiIDMap.Add({ tablename , output });
		}
	}

	return true;
}

void FGoogleSheetDataTableModule::UpdateSheetIDToFile(FString& sheetid, const FString& path, const FString& tablename)
{
	FString contentDir = FPaths::ProjectContentDir();
	FString fileData;
	
	if (FFileHelper::LoadFileToString(fileData, *(contentDir + path)))
	{
		FGoogleApiSetting ApiSetting;
		TSharedPtr<FJsonObject> googlesetting;
		FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(fileData), googlesetting);
		if (!FJsonObjectConverter::JsonObjectToUStruct<FGoogleApiSetting>(googlesetting.ToSharedRef(), &ApiSetting))
		{
			UE_LOG(LogTemp, Warning, TEXT("Can not open GoogleApiSetting"));
			return;
		}

		if (ApiSetting.googleApiIDMap.Contains(tablename))
		{
			ApiSetting.googleApiIDMap[tablename] = sheetid;
		}
		else
		{
			ApiSetting.googleApiIDMap.Add({ tablename , sheetid });
		}

		FString jsonString;
		TSharedPtr<FJsonObject> JsonObject = FJsonObjectConverter::UStructToJsonObject(ApiSetting);
		FJsonSerializer::Serialize(JsonObject.ToSharedRef(), TJsonWriterFactory<>::Create(&jsonString, 0));

		FFileHelper::SaveStringToFile(jsonString, *(contentDir + FILE_PATH));
	}


}

void FGoogleSheetDataTableModule::ChangeSheetID(const FText& InValue)
{
	_cacheSheetId = InValue.ToString();

	FString dataTableName;
	_selectedDataTable->GetName(dataTableName);
	UpdateSheetIDToFile(_cacheSheetId, FILE_PATH, dataTableName);
}

void FGoogleSheetDataTableModule::ChangeSetSheetID(const FText& InValue)
{
	_cacheSetSheetId = InValue.ToString();
}

void FGoogleSheetDataTableModule::CloseWindow()
{
	if (_windowPtr.IsValid())
	{
		_windowPtr->RequestDestroyWindow();
	}
}

void FGoogleSheetDataTableModule::UpdateDataTableFromGoogleSheet()
{
	_googleSheetApi = NewObject<UGoogleSheetApi>(UGoogleSheetApi::StaticClass());
	_googleSheetApi->OnResponseDelegate.BindRaw(this, &FGoogleSheetDataTableModule::OnApiResponse);
	_googleSheetApi->SendRequest(_cacheSheetId);
}

FReply FGoogleSheetDataTableModule::OnOKClicked()
{
	UpdateDataTableFromGoogleSheet();

	CloseWindow();

	return FReply::Handled();
}

FReply FGoogleSheetDataTableModule::OnSaveClicked()
{
	FString dataTableName;
	_selectedDataTable->GetName(dataTableName);
	_cacheSheetId = _cacheSetSheetId;
	UpdateSheetIDToFile(_cacheSheetId, FILE_PATH, dataTableName);

	CloseWindow();

	return FReply::Handled();
}

FReply FGoogleSheetDataTableModule::OnCancelClicked()
{
	CloseWindow();

	return FReply::Handled();
}

void FGoogleSheetDataTableModule::OnApiResponse(FString Response)
{
	UDataTableFunctionLibrary::FillDataTableFromCSVString(_selectedDataTable, Response);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGoogleSheetDataTableModule, GoogleSheetDataTable)