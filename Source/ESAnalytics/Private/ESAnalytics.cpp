// Copyright (c) Guillermo Valdez 2026

#include "ESAnalytics.h"

#include "ESAnalyticsSettings.h"
#include "Containers/Ticker.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "JsonObjectConverter.h"

#define LOCTEXT_NAMESPACE "FESAnalyticsModule"

DEFINE_LOG_CATEGORY_STATIC(LogESAnalytics, Log, Log);

void FESAnalyticsModule::StartupModule()
{
	UESAnalyticsSettings* Settings = GetMutableDefault<UESAnalyticsSettings>();
	if (!IsValid(Settings))
	{
		return;
	}

	// Set up periodic flush based on config settings
	if (Settings->FlushInterval > 0.0f)
	{
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&FESAnalyticsModule::PeriodicFlush), Settings->FlushInterval);
	}
}

void FESAnalyticsModule::ShutdownModule()
{
	// Remove periodic flush ticker
	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
	}
}

bool FESAnalyticsModule::PeriodicFlush(float DeltaTime)
{
	// Ensure that feature is still enabled before flushing analytics
	const UESAnalyticsSettings* Settings = GetDefault<UESAnalyticsSettings>();
	if (!IsValid(Settings) || !Settings->bEnabled)
	{
		return true;
	}

	FlushAnalytics();

	return true;
}

void FESAnalyticsModule::EmitAnalytic(const FESAnalyticEvent& AnalyticEvent)
{
	if (!ensureAlwaysMsgf(IsInGameThread(), TEXT("Tried calling 'EmitAnalytic' outside of game thread! Discarding")))
	{
		return;
	}

	// Ensure that feature is still enabled before adding analytics
	const UESAnalyticsSettings* Settings = GetDefault<UESAnalyticsSettings>();
	if (!IsValid(Settings) || !Settings->bEnabled)
	{
		return;
	}

	FESAnalyticsModule* Module = FModuleManager::Get().LoadModulePtr<FESAnalyticsModule>(TEXT("ESAnalytics"));
	if (!Module)
	{
		return;
	}

	// Add new event and set timestamp/id
	FESAnalyticEvent& InsertedEvent = Module->PendingEvents.Add_GetRef(AnalyticEvent);
	InsertedEvent.Timestamp = FDateTime::UtcNow().ToIso8601();
	InsertedEvent.Id = FGuid::NewGuid();

	// Parse any entity-related data here
	if (IsValid(InsertedEvent.Entity))
	{
		// Fallback to entity name
		if (InsertedEvent.EntityName.IsEmpty())
		{
			InsertedEvent.EntityName = InsertedEvent.Entity->GetName();
		}

		// Populate coordinates from entity
		FVector Loc = InsertedEvent.Entity->GetActorLocation();

		InsertedEvent.IsSpatial = true;
		InsertedEvent.LocX = Loc.X;
		InsertedEvent.LocY = Loc.Y;
		InsertedEvent.LocZ = Loc.Z;
	}

	UE_LOG(LogESAnalytics, Verbose, TEXT("Emitting analytic event %s!"), *InsertedEvent.Name);

	// Immediately flush analytics if the payload is at max size
	if (Module->PendingEvents.Num() >= Settings->MaxPayloadSize)
	{
		FlushAnalytics();
	}
}

void FESAnalyticsModule::FlushAnalytics()
{
	// Ensure that feature is still enabled before flushing analytics
	const UESAnalyticsSettings* Settings = GetDefault<UESAnalyticsSettings>();
	if (!IsValid(Settings) || !Settings->bEnabled)
	{
		return;
	}

	FESAnalyticsModule* Module = FModuleManager::Get().LoadModulePtr<FESAnalyticsModule>(TEXT("ESAnalytics"));
	if (!Module)
	{
		return;
	}

	if (Module->PendingEvents.IsEmpty())
	{
		return;
	}

	// Construct request string using all objects in PendingEvents
	FString RequestString;
	for (const FESAnalyticEvent& Event : Module->PendingEvents)
	{
		UE_LOG(LogESAnalytics, Verbose, TEXT("Flushing analytic event %s!"), *Event.Name);

		FString JsonString;
		if (FJsonObjectConverter::UStructToJsonObjectString(Event, JsonString, 0, CPF_Deprecated | CPF_Transient | CPF_SkipSerialization, 0, nullptr, false))
		{
			RequestString += FString::Printf(TEXT("{\"index\":{\"_index\":\"%s\",\"_id\":\"%s\"}}\n%s\n"),
				Event.IndexOverride.IsEmpty() ? *Settings->Index : *Event.IndexOverride,
				*Event.Id.ToString(EGuidFormats::Short),
				*JsonString);
		}
	}

	// Construct HTTP request
	TSharedPtr<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->SetVerb("POST");
	Request->SetURL(Settings->Endpoint + "/_bulk");

	if (!Settings->ApiKey.IsEmpty())
	{
		Request->SetHeader("Authorization", FString::Printf(TEXT("ApiKey %s"), *Settings->ApiKey));
	}

	Request->SetHeader("Content-Type", "application/x-ndjson");
	Request->SetHeader("Accept", "*/*");
	Request->SetHeader("Accept-Encoding", "gzip, deflate, br");
	Request->SetContentAsString(RequestString);

	FGuid RequestId = FGuid::NewGuid();
	UE_LOG(LogESAnalytics, Log, TEXT("[%s] Flushing %d analytic events!"), *RequestId.ToString(EGuidFormats::Short), Module->PendingEvents.Num());
	Request->OnProcessRequestComplete().BindLambda(
		[RequestId](FHttpRequestPtr, FHttpResponsePtr InResponse, bool bSuccess)
		{
			if (!bSuccess || !InResponse.IsValid())
			{
				UE_LOG(LogESAnalytics, Error, TEXT("[%s] Failed to emit analytics!"), *RequestId.ToString(EGuidFormats::Short));
			}
			else
			{
				UE_LOG(LogESAnalytics, Log, TEXT("[%s] Analytic events successfully emitted!\n %s"), *RequestId.ToString(EGuidFormats::Short), *InResponse->GetContentAsString());
			}
		});

	Request->ProcessRequest();
	Module->PendingEvents.Empty();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FESAnalyticsModule, ESAnalytics)