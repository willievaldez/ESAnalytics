// Copyright (c) Guillermo Valdez 2026

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StructUtils/InstancedStruct.h"

#include "Modules/ModuleManager.h"

#include "ESAnalytics.generated.h"

USTRUCT(BlueprintType)
struct FESAnalyticEvent
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	// Associate an actor with the emitted event
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SkipSerialization)
	TObjectPtr<AActor> Entity;

	// Associate an entity name with the emitted event. If not specified, will use Entity->GetFName()
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EntityName;

	// Associate any additional data with the emitted event
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FString> Metadata;

	// Override the elastic search index if specified. Not serialized into event JSON
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SkipSerialization)
	FString IndexOverride;

	// These fields are not blueprint exposed as the are automatically set via code
	UPROPERTY()
	FString Timestamp;

	UPROPERTY()
	float LocX = 0.0f;

	UPROPERTY()
	float LocY = 0.0f;

	UPROPERTY()
	float LocZ = 0.0f;

	UPROPERTY()
	bool IsSpatial = false;

	UPROPERTY(SkipSerialization)
	FGuid Id;
};

class FESAnalyticsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	friend class UESAnalytics;

	static bool PeriodicFlush(float DeltaTime);
	static void EmitAnalytic(const FESAnalyticEvent& AnalyticEvent);
	static void FlushAnalytics();

	FTSTicker::FDelegateHandle TickHandle;
	TArray<FESAnalyticEvent> PendingEvents;
};

UCLASS(meta = (ScriptName = "ESAnalytics"))
class UESAnalytics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	static void EmitAnalytic(const FESAnalyticEvent& AnalyticEvent) {
		FESAnalyticsModule::EmitAnalytic(AnalyticEvent);
	}
};
