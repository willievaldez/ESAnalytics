// Copyright (c) Guillermo Valdez 2026

#include "Engine/DeveloperSettings.h"

#include "ESAnalyticsSettings.generated.h"

UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Analytics"))
class UESAnalyticsSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// Whether or not analytic emission is enabled
	UPROPERTY(Config, EditAnywhere, Category = Analytics, meta = (ConsoleVariable = "Analytics.Enabled"))
	bool bEnabled = true;

	// The default elastic search index that events will be emitted to
	UPROPERTY(Config, EditAnywhere, Category = Analytics, meta = (ConsoleVariable = "Analytics.Endpoint"))
	FString Endpoint = "http://localhost:9200";

	// The default elastic search index that events will be emitted to
	UPROPERTY(Config, EditAnywhere, Category = Analytics, meta = (ConsoleVariable = "Analytics.ApiKey"))
	FString ApiKey;

	// The default elastic search index that events will be emitted to
	UPROPERTY(Config, EditAnywhere, Category = Analytics, meta = (ConsoleVariable = "Analytics.Index"))
	FString Index;

	// The maximum number of events to batch before automatically flushing analytics events
	UPROPERTY(Config, EditAnywhere, Category = Analytics, meta = (ConsoleVariable = "Analytics.MaxPayloadSize"))
	int32 MaxPayloadSize = 10;

	// The interval at which analytics will automatically be flushed
	UPROPERTY(Config, EditAnywhere, Category = Analytics)
	float FlushInterval = 10.0f;
};
