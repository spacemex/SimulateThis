// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Components/SimWorldSchedulerComponent.h"
#include "World/Components/SimWorldClockComponent.h"
#include "World/SimWorldSaveGame.h"


USimWorldSchedulerComponent::USimWorldSchedulerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void USimWorldSchedulerComponent::StartScheduler()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bRunning)return;

	WorldClock = GetOwner()->FindComponentByClass<USimWorldClockComponent>();

	if (!WorldClock || !WorldClock->IsClockReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("Scheduler: WorldClock is not ready."));
		return;
	}

	const UWorld* World = GetWorld();

	if (!World) return;

	if (!bHasProcessedDayState)
	{
		LastProcessedDay = FMath::FloorToInt64(WorldClock->GetGameTimeSeconds() / 86400.0);
		bHasProcessedDayState = true;
	}

	bRunning = true;

	World->GetTimerManager().SetTimer(SchedulerTimer, this, &USimWorldSchedulerComponent::ProcessPendingDays, 0.25F,
	                                  true);
	UE_LOG(LogTemp, Log, TEXT("Scheduler: Started from day %lld."), static_cast<long long>(LastProcessedDay + 1));
}

void USimWorldSchedulerComponent::ProcessPendingDays()
{
	if (!bRunning || !WorldClock || !WorldClock->IsClockReady()) return;

	const int64 CurrentDay = FMath::FloorToInt64(WorldClock->GetGameTimeSeconds() / 86400.0);

	if (CurrentDay < LastProcessedDay)
	{
		LastProcessedDay = CurrentDay;
		return;
	}

	constexpr int32 MaxDaysPerUpdate = 32;

	int32 Processed = 0;

	while (LastProcessedDay < CurrentDay && Processed < MaxDaysPerUpdate)
	{
		++LastProcessedDay;
		++Processed;

		const int64 NewDay = LastProcessedDay + 1;
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

FSimSchedulerSaveData USimWorldSchedulerComponent::CreateSaveData() const
{
	FSimSchedulerSaveData Data;

	if (!GetOwner() || !GetOwner()->HasAuthority() || !bHasProcessedDayState) return Data;

	Data.bInitialized = true;
	Data.LastProcessedDay = LastProcessedDay;

	return Data;
}

bool USimWorldSchedulerComponent::RestoreFromSaveData(const FSimSchedulerSaveData& SaveData, double SavedGameSeconds)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || bRunning || !FMath::IsFinite(SavedGameSeconds) || SavedGameSeconds
		< 0.0)
		return false;

	const int64 SavedCurrentDay = FMath::FloorToInt64(SavedGameSeconds / 86400.0);

	if (SaveData.bInitialized)
	{
		if (SaveData.LastProcessedDay < 0 || SaveData.LastProcessedDay > SavedCurrentDay)
		{
			return false;
		}

		LastProcessedDay = SaveData.LastProcessedDay;
	}
	else
	{
		LastProcessedDay = SavedCurrentDay;
	}

	bHasProcessedDayState = true;

	UE_LOG(LogTemp, Log, TEXT("Scheduler: Restored checkpoint at day %lld."),
	       static_cast<long long>(LastProcessedDay + 1));
	return true;
}

int64 USimWorldSchedulerComponent::GetLastProcessedDayNumber() const
{
	return bHasProcessedDayState ? LastProcessedDay + 1 : 0;
}

void USimWorldSchedulerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopScheduler();

	Super::EndPlay(EndPlayReason);
}
