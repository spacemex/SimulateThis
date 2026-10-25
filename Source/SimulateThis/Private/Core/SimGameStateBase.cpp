// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SimGameStateBase.h"

#include "Kismet/GameplayStatics.h"
#include "World/SimWorldClockComponent.h"
#include "World/SimCalendarEventComponent.h"
#include "World/SimWorldSchedulerComponent.h"
#include "World/SimWeatherComponent.h"
#include "World/SimWorldSaveGame.h"

ASimGameStateBase::ASimGameStateBase()
{
	WorldClock = CreateDefaultSubobject<USimWorldClockComponent>(TEXT("WorldClock"));
	CalendarEvents = CreateDefaultSubobject<USimCalendarEventComponent>(TEXT("CalendarEvents"));
	WorldScheduler = CreateDefaultSubobject<USimWorldSchedulerComponent>(TEXT("WorldScheduler"));
	WorldWeather = CreateDefaultSubobject<USimWeatherComponent>(TEXT("WorldWeather"));
}


bool ASimGameStateBase::SaveWorld()
{
	if (!HasAuthority()) return false;

	if (!bPersistenceReady || !WorldClock || !WorldScheduler || SaveSlotName.IsEmpty())
	{
		OnSave.Broadcast(false);
		return false;
	}

	USimWorldSaveGame* SaveData = Cast<USimWorldSaveGame>(
		UGameplayStatics::CreateSaveGameObject(USimWorldSaveGame::StaticClass()));

	if (!SaveData)
	{
		OnSave.Broadcast(false);
		return false;
	}

	SaveData->SaveVersion = 2;

	SaveData->ClockData = WorldClock->CreateSaveData();
	SaveData->SchedulerData = WorldScheduler->CreateSaveData();

	if (!SaveData->SchedulerData.bInitialized)
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Scheduler checkpoint unavailable."));
		OnSave.Broadcast(false);
		return false;
	}

	const bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveData, SaveSlotName, 0);

	if (bSuccess)
	{
		UE_LOG(LogTemp, Log, TEXT("Persistence: World saved to %s."), *SaveSlotName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Failed to save world."));
	}

	OnSave.Broadcast(bSuccess);
	return bSuccess;
}


void ASimGameStateBase::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;

	GetWorldTimerManager().SetTimerForNextTick(this, &ASimGameStateBase::InitializeWorldPersistence);
}

void ASimGameStateBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(AutoSaveTimer);

	Super::EndPlay(EndPlayReason);
}

void ASimGameStateBase::InitializeWorldPersistence()
{
	if (!HasAuthority()) return;

	if (!WorldClock)
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: WorldClock is missing."));
		return;
	}

	if (!WorldClock->IsClockReady())
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ASimGameStateBase::InitializeWorldPersistence);
		return;
	}

	if (SaveSlotName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Save slot name is empty."));
		return;
	}

	const bool bSaveExists = UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);

	if (bSaveExists)
	{
		if (!LoadWorld())
		{
			UE_LOG(LogTemp, Error, TEXT("Persistence: Load failed. Autosave disabled."));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Persistence: Starting a new world."));
	}

	bPersistenceReady = true;

	if (WorldWeather)
	{
		WorldWeather->InitializeWeather();
	}

	if (WorldScheduler)
	{
		WorldScheduler->StartScheduler();
	}

	if (AutoSaveIntervalSeconds > 0.0f)
	{
		GetWorldTimerManager().SetTimer(AutoSaveTimer, this, &ASimGameStateBase::AutoSaveWorld, AutoSaveIntervalSeconds,
		                                true);
	}
}


bool ASimGameStateBase::LoadWorld()
{
	if (!HasAuthority() || !WorldClock || !WorldScheduler) return false;

	USimWorldSaveGame* SaveData = Cast<USimWorldSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));

	if (!SaveData) return false;

	if (SaveData->SaveVersion != 1 && SaveData->SaveVersion != 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Unsupported save version %d."), SaveData->SaveVersion);
		return false;
	}

	FSimSchedulerSaveData SchedulerSnapshot;

	if (SaveData->SaveVersion == 2)
	{
		SchedulerSnapshot = SaveData->SchedulerData;

		if (!SchedulerSnapshot.bInitialized)
		{
			UE_LOG(LogTemp, Error, TEXT("Persistence: Scheduler checkpoint missing."));
			return false;
		}
	}

	if (!WorldScheduler->RestoreFromSaveData(SchedulerSnapshot, SaveData->ClockData.GameSeconds))
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Invalid scheduler checkpoint."));
		return false;
	}

	if (!WorldClock->RestoreFromSaveData(SaveData->ClockData))
	{
		UE_LOG(LogTemp, Error, TEXT("Persistence: Failed to restore clock."));
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Persistence: Loaded version %d from %s."), SaveData->SaveVersion, *SaveSlotName);
	return true;
}

void ASimGameStateBase::AutoSaveWorld()
{
	SaveWorld();
}
