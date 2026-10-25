// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SimWorldClockComponent.h"

#include "GameFramework/GameStateBase.h"
#include "Net/UnrealNetwork.h"
#include "World/SimWorldSaveGame.h"

USimWorldClockComponent::USimWorldClockComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void USimWorldClockComponent::BeginPlay()
{
	Super::BeginPlay();

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(CalendarPollHandle, this, &USimWorldClockComponent::CheckCalendarEvents,
		                                  0.25F, true);
	}

	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	ClockState.GameSecondsAtAnchor = FMath::Clamp(StartingHour, 0, 23) * 3600;
	ClockState.ServerSecondsAtAnchor = GetServerTimeSeconds();
	ClockState.TimeScale = FMath::Max(0.F, InitialTimeScale);
	ClockState.StartingYear = FMath::Max(1, InitialYear);
	ClockState.DaysPerSeason = FMath::Max(1, SeasonLengthInDays);
	ClockState.bInitialized = true;

	GetOwner()->ForceNetUpdate();
}

void USimWorldClockComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USimWorldClockComponent, ClockState);
}

void USimWorldClockComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CalendarPollHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void USimWorldClockComponent::OnRep_ClockState(const FSimClockState& PreviousState)
{
	if (ClockState.Revision != PreviousState.Revision)
	{
		bHasClockSample = false;
		OnClockRestored.Broadcast();
	}
}

double USimWorldClockComponent::GetServerTimeSeconds() const
{
	const AGameStateBase* GameState = Cast<AGameStateBase>(GetOwner());

	return GameState ? GameState->GetServerWorldTimeSeconds() : 0.0;
}

void USimWorldClockComponent::CheckCalendarEvents()
{
	if (!IsClockReady()) return;

	const double GameSeconds = GetGameTimeSeconds();
	const int64 CurrentHour = FMath::FloorToInt64(GameSeconds / 3600.0);
	const int64 CurrentMinute = FMath::FloorToInt64(GameSeconds / 60.0);
	const int64 CurrentDay = FMath::FloorToInt64(GameSeconds / 86400.0);

	if (!bHasClockSample)
	{
		LastObservedHour = CurrentHour;
		LastObservedMinute = CurrentMinute;
		LastObservedDay = CurrentDay;
		bHasClockSample = true;
		return;
	}

	if (CurrentDay != LastObservedDay)
	{
		const FSimCalendarDate PreviousDate = BuildCalendarDate(LastObservedDay);
		const FSimCalendarDate CurrentDate = BuildCalendarDate(CurrentDay);

		if (CurrentDate.Year != PreviousDate.Year)
		{
			OnYearChanged.Broadcast(CurrentDate.Year);
		}

		if (CurrentDate.Season != PreviousDate.Season || CurrentDate.Year != PreviousDate.Year)
		{
			OnSeasonChanged.Broadcast(CurrentDate.Season, CurrentDate.Year);
		}

		OnDayChanged.Broadcast(GetDay());
	}

	if (CurrentHour != LastObservedHour)
	{
		OnHourChanged.Broadcast(GetHour());
	}

	if (CurrentMinute != LastObservedMinute)
	{
		OnMinuteChanged.Broadcast(GetMinute());
	}

	LastObservedHour = CurrentHour;
	LastObservedMinute = CurrentMinute;
	LastObservedDay = CurrentDay;
}

FSimCalendarDate USimWorldClockComponent::BuildCalendarDate(int64 AbsoluteDay) const
{
	FSimCalendarDate Date;

	if (!IsClockReady()) return Date;

	const int64 DaysPerSeason = FMath::Max(1, ClockState.DaysPerSeason);
	const int64 DaysPerYear = DaysPerSeason * 4;
	const int64 YearOffset = AbsoluteDay / DaysPerYear;

	Date.Year = static_cast<int32>(ClockState.StartingYear + YearOffset);

	const int64 DayInYear = AbsoluteDay % DaysPerYear;

	Date.DayOfYear = static_cast<int32>(DayInYear + 1);

	const int32 SeasonIndex = static_cast<int32>(DayInYear / DaysPerSeason);

	Date.Season = static_cast<ESimSeason>(SeasonIndex);
	Date.DayOfSeason = static_cast<int32>(DayInYear % DaysPerSeason) + 1;
	Date.bIsValid = true;
	return Date;
}

bool USimWorldClockComponent::IsClockReady() const
{
	return ClockState.bInitialized;
}

double USimWorldClockComponent::GetGameTimeSeconds() const
{
	if (!IsClockReady()) return 0.0;

	const double ElapsedSeconds = FMath::Max(0.0, GetServerTimeSeconds() - ClockState.ServerSecondsAtAnchor);

	return ClockState.GameSecondsAtAnchor + ElapsedSeconds * ClockState.TimeScale;
}

int32 USimWorldClockComponent::GetDay() const
{
	if (!IsClockReady()) return 0;

	return FMath::FloorToInt(GetGameTimeSeconds() / 86400.0) + 1;
}

int32 USimWorldClockComponent::GetHour() const
{
	const int64 TotalHours = FMath::FloorToInt64(GetGameTimeSeconds() / 3600.0);

	return static_cast<int32>(TotalHours % 24);
}

int32 USimWorldClockComponent::GetMinute() const
{
	const int64 TotalMinutes = FMath::FloorToInt64(GetGameTimeSeconds() / 60.0);

	return static_cast<int32>(TotalMinutes % 60);
}

float USimWorldClockComponent::GetDayProgress() const
{
	if (!IsClockReady()) return 0.0F;

	constexpr double SecondsPerDay = 86400.0;

	const double SecondsToday = FMath::Fmod(GetGameTimeSeconds(), SecondsPerDay);
	return static_cast<float>(SecondsToday / SecondsPerDay);
}

FSimCalendarDate USimWorldClockComponent::GetCalendarDate() const
{
	if (!IsClockReady()) return FSimCalendarDate();

	const int64 AbsoluteDay = FMath::FloorToInt64(GetGameTimeSeconds() / 86400.0);
	return BuildCalendarDate(AbsoluteDay);
}

int32 USimWorldClockComponent::GetDaysPerSeason() const
{
	if (!IsClockReady()) return 0;

	return FMath::Max(1, ClockState.DaysPerSeason);
}

void USimWorldClockComponent::SetTimeScale(float NewTimeScale)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsClockReady()) return;

	ClockState.GameSecondsAtAnchor = GetGameTimeSeconds();
	ClockState.ServerSecondsAtAnchor = GetServerTimeSeconds();
	ClockState.TimeScale = FMath::Max(0.F, NewTimeScale);

	GetOwner()->ForceNetUpdate();
}

FSimClockSaveData USimWorldClockComponent::CreateSaveData() const
{
	FSimClockSaveData Data;

	if (!GetOwner() || !GetOwner()->HasAuthority() || !IsClockReady()) return Data;

	Data.GameSeconds = GetGameTimeSeconds();
	Data.TimeScale = ClockState.TimeScale;
	Data.StartingYear = ClockState.StartingYear;
	Data.DaysPerSeason = ClockState.DaysPerSeason;

	return Data;
}

bool USimWorldClockComponent::RestoreFromSaveData(const FSimClockSaveData& SaveData)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;

	if (!FMath::IsFinite(SaveData.GameSeconds) || SaveData.GameSeconds < 0.0 || !FMath::IsFinite(SaveData.TimeScale)
		|| SaveData.TimeScale < 0.0F || SaveData.StartingYear < 1 || SaveData.DaysPerSeason < 1)
		return false;

	ClockState.GameSecondsAtAnchor = SaveData.GameSeconds;
	ClockState.ServerSecondsAtAnchor = GetServerTimeSeconds();
	ClockState.TimeScale = SaveData.TimeScale;
	ClockState.StartingYear = SaveData.StartingYear;
	ClockState.DaysPerSeason = SaveData.DaysPerSeason;
	ClockState.bInitialized = true;

	++ClockState.Revision;

	bHasClockSample = false;

	OnClockRestored.Broadcast();

	GetOwner()->ForceNetUpdate();

	return true;
}
