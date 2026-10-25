// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "SimWorldSchedulerComponent.generated.h"

class USimWorldClockComponent;
struct FSimSchedulerSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimSumulationDayStarted, int64, NewDay);

UCLASS(ClassGroup=(Simulation))
class SIMULATETHIS_API USimWorldSchedulerComponent : public UActorComponent
{
private:
	GENERATED_BODY()

public:
	USimWorldSchedulerComponent();

	UPROPERTY(BlueprintAssignable, Category="Simulation|Scheduler")
	FSimSumulationDayStarted OnSimulationDayStarted;

	void StartScheduler();
	void StopScheduler();

	FSimSchedulerSaveData CreateSaveData() const;

	bool RestoreFromSaveData(const FSimSchedulerSaveData& SaveData, double SavedGameSeconds);

	UFUNCTION(BlueprintPure, Category="Simulation|Scheduler")
	int64 GetLastProcessedDayNumber() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<USimWorldClockComponent> WorldClock;

	FTimerHandle SchedulerTimer;
	int64 LastProcessedDay = -1;
	bool bRunning = false;
	bool bHasProcessedDayState = false;

	void ProcessPendingDays();
};
