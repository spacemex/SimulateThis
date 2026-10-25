// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SimWorldSchedulerComponent.h"

#include "World/SimWorldClockComponent.h"


USimWorldSchedulerComponent::USimWorldSchedulerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USimWorldSchedulerComponent::StartScheduler()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bRunning) return;

	WorldClock = GetOwner()->FindComponentByClass<USimWorldClockComponent>();

	if (!WorldClock || !WorldClock->IsClockReady())
	{
		UE_LOG(LogTemp, Error, TEXT("Scheduler: Clock is not ready."));
		return;
	}

	LastDayProcessedDay = FMath::FloorToInt64(WorldClock->GetGameTimeSeconds() / 86400.0);

	const UWorld* World = GetWorld();

	if (!World) return;

	bRunning = true;

	World->GetTimerManager().SetTimer(SchedulerTimer, this, &USimWorldSchedulerComponent::ProcessPendingDays, 0.25F,
	                                  true);

	UE_LOG(LogTemp, Log, TEXT("Scheduler: Started at day %lld"), static_cast<long long>(LastDayProcessedDay + 1));
}

void USimWorldSchedulerComponent::ProcessPendingDays()
{
	if (!bRunning || !WorldClock || !WorldClock->IsClockReady()) return;

	const int64 CurrentDay = FMath::FloorToInt64(WorldClock->GetGameTimeSeconds() / 86400.0);

	if (CurrentDay < LastDayProcessedDay)
	{
		LastDayProcessedDay = CurrentDay;
		return;
	}

	constexpr int32 MaxDaysPerUpdate = 32;

	int32 Processed = 0;

	while (LastDayProcessedDay < CurrentDay && Processed < MaxDaysPerUpdate)
	{
		++LastDayProcessedDay;
		++Processed;

		const int64 NewDay = LastDayProcessedDay + 1;
		OnSimulationDayStarted.Broadcast(NewDay);
	}
}

void USimWorldSchedulerComponent::StopScheduler()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SchedulerTimer);
	}

	bRunning = false;
}

void USimWorldSchedulerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopScheduler();

	Super::EndPlay(EndPlayReason);
}
