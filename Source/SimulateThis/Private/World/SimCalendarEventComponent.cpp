// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SimCalendarEventComponent.h"

#include "World/SimCalendarEventData.h"
#include "World/SimWorldClockComponent.h"
#include "World/SimCalendarTypes.h"

#include "Net/UnrealNetwork.h"


USimCalendarEventComponent::USimCalendarEventComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USimCalendarEventComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	WorldClock = GetOwner()->FindComponentByClass<USimWorldClockComponent>();

	if (!WorldClock)
	{
		UE_LOG(LogTemp, Error, TEXT("CalendarEventComponent: WorldClock not found."));
		return;
	}

	WorldClock->OnDayChanged.AddUniqueDynamic(this, &USimCalendarEventComponent::HandleDayChanged);
	WorldClock->OnClockRestored.AddUniqueDynamic(this, &USimCalendarEventComponent::HandleClockRestored);

	TryInitialize();
}

void USimCalendarEventComponent::TryInitialize()
{
	if (!WorldClock || !WorldClock->IsClockReady())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(InitializationTimer, this, &USimCalendarEventComponent::TryInitialize,
			                                  0.25F, false);
		}
		return;
	}

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitializationTimer);
	}

	EvaluateActiveEvents();
}

void USimCalendarEventComponent::HandleDayChanged(int32 NewDay)
{
	EvaluateActiveEvents();
}

void USimCalendarEventComponent::HandleClockRestored()
{
	EvaluateActiveEvents();
}

void USimCalendarEventComponent::EvaluateActiveEvents()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !WorldClock || !WorldClock->IsClockReady()) return;

	const FSimCalendarDate Date = WorldClock->GetCalendarDate();

	if (!Date.bIsValid) return;

	const int32 DaysPerSeason = WorldClock->GetDaysPerSeason();

	if (DaysPerSeason <= 0) return;

	const int32 DaysPerYear = DaysPerSeason * 4;
	const int32 CurrentDayIndex = Date.DayOfYear - 1;

	TArray<FName> NewActiveIds;

	for (const auto Event : EventDefinitions)
	{
		if (!Event || !Event->bEnabled || Event->EventId.IsNone()) continue;

		if (Event->StartDayOfSeason < 1 || Event->StartDayOfSeason > DaysPerSeason || Event->DurationDays < 1 || Event->
			DurationDays > DaysPerYear)
			continue;

		const int32 SeasonIndex = static_cast<int32>(Event->StartSeason);
		const int32 StartDayIndex = SeasonIndex * DaysPerSeason + Event->StartDayOfSeason - 1;
		const int32 DaysSinceStart = (CurrentDayIndex - StartDayIndex + DaysPerYear) % DaysPerYear;

		if (DaysSinceStart < Event->DurationDays)
		{
			NewActiveIds.AddUnique(Event->EventId);
		}
	}

	if (NewActiveIds == ActiveEventIds) return;

	const TArray<FName> PreviousIds = ActiveEventIds;

	ActiveEventIds = MoveTemp(NewActiveIds);

	BroadcastChanges(PreviousIds, ActiveEventIds);

	GetOwner()->ForceNetUpdate();
}

void USimCalendarEventComponent::BroadcastChanges(const TArray<FName>& Previous, const TArray<FName>& Current)
{
	for (const auto& EventId : Previous)
	{
		if (!Current.Contains(EventId))
		{
			OnEventEnded.Broadcast(EventId);
		}
	}

	for (const auto& EventId : Current)
	{
		if (!Previous.Contains(EventId))
		{
			OnEventStarted.Broadcast(EventId);
		}
	}
}

void USimCalendarEventComponent::OnRep_ActiveEventIds(const TArray<FName>& PreviousEventIds)
{
	BroadcastChanges(PreviousEventIds, ActiveEventIds);
}

TArray<FName> USimCalendarEventComponent::GetActiveEventIds() const
{
	return ActiveEventIds;
}

bool USimCalendarEventComponent::IsEventActive(FName EventId) const
{
	return ActiveEventIds.Contains(EventId);
}

USimCalendarEventData* USimCalendarEventComponent::FindEventDefinition(FName EventId) const
{
	for (auto Event : EventDefinitions)
	{
		if (Event && Event->EventId == EventId) return Event;
	}
	return nullptr;
}

void USimCalendarEventComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USimCalendarEventComponent, ActiveEventIds);
}

void USimCalendarEventComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WorldClock)
	{
		WorldClock->OnDayChanged.RemoveDynamic(this, &USimCalendarEventComponent::HandleDayChanged);
		WorldClock->OnClockRestored.RemoveDynamic(this, &USimCalendarEventComponent::HandleClockRestored);
	}

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(InitializationTimer);
	}

	Super::EndPlay(EndPlayReason);
}
