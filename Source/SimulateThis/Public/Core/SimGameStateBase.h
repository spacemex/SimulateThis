// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TimerManager.h"
#include "SimGameStateBase.generated.h"

class USimWorldClockComponent;
class USimCalendarEventComponent;
class USimWorldSchedulerComponent;
class USimWeatherComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimGameOnSave, bool, bSuccess);

UCLASS()
class SIMULATETHIS_API ASimGameStateBase : public AGameStateBase
{
private:
	GENERATED_BODY()

public:
	ASimGameStateBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation|Time")
	TObjectPtr<USimWorldClockComponent> WorldClock;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation|Calendar")
	TObjectPtr<USimCalendarEventComponent> CalendarEvents;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation|Scheduler")
	TObjectPtr<USimWorldSchedulerComponent> WorldScheduler;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Simulation|Weather")
	TObjectPtr<USimWeatherComponent> WorldWeather;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Persistence")
	FSimGameOnSave OnSave;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Persistence")
	FString SaveSlotName = TEXT("World_01");

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Persistence", meta=(ClampMin="0.0"))
	float AutoSaveIntervalSeconds = 120.0F;

	UFUNCTION(BlueprintCallable, Category="Simulation|Persistence")
	bool SaveWorld();

	UFUNCTION(BlueprintPure, Category="Simulation|Persistence")
	FORCEINLINE bool IsWorldPersistenceReady() const
	{
		return bPersistenceReady;
	}

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void InitializeWorldPersistence();
	bool LoadWorld();
	void AutoSaveWorld();
	FTimerHandle AutoSaveTimer;
	bool bPersistenceReady = false;
};
