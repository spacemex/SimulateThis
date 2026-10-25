// Fill out your copyright notice in the Description page of Project Settings.


#include "World/SimWeatherComponent.h"
#include "Math/RandomStream.h"
#include "Net/UnrealNetwork.h"
#include "World/SimWeatherData.h"
#include "World/SimWorldClockComponent.h"


USimWeatherComponent::USimWeatherComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

FSimWeatherState USimWeatherComponent::GetWeatherState() const
{
	return WeatherState;
}

FName USimWeatherComponent::GetCurrentWeatherId() const
{
	return WeatherState.CurrentWeatherId;
}

FName USimWeatherComponent::GetTomorrowWeatherId() const
{
	return WeatherState.TomorrowWeatherId;
}

USimWeatherData* USimWeatherComponent::FindWeatherDefinition(FName WeatherId) const
{
	for (auto Definition : WeatherDefinitions)
	{
		if (Definition && Definition->WeatherId == WeatherId)
		{
			return Definition;
		}
	}

	return nullptr;
}

void USimWeatherComponent::InitializeWeather()
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	WorldClock = GetOwner()->FindComponentByClass<USimWorldClockComponent>();

	if (!WorldClock || !WorldClock->IsClockReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("Weather: WorldClock not ready."));
		return;
	}

	WorldClock->OnDayChanged.AddUniqueDynamic(this, &USimWeatherComponent::HandleDayChanged);
	WorldClock->OnClockRestored.AddUniqueDynamic(this, &USimWeatherComponent::HandleClockRestored);

	RefreshWeather();
}

void USimWeatherComponent::HandleDayChanged(int32 NewDay)
{
	RefreshWeather();
}

void USimWeatherComponent::HandleClockRestored()
{
	RefreshWeather();
}

FName USimWeatherComponent::GetWeatherIdForDay(int64 AbsoluteDay) const
{
	if (!WorldClock || !WorldClock->IsClockReady() || AbsoluteDay < 0) return NAME_None;

	const int64 DaysPerSeason = WorldClock->GetDaysPerSeason();

	if (DaysPerSeason <= 0) return NAME_None;

	const int32 SeasonIndex = static_cast<int32>((AbsoluteDay / DaysPerSeason) % 4);
	const ESimSeason Season = static_cast<ESimSeason>(SeasonIndex);

	struct FWeightedWeather
	{
		FName Id;
		double Weight = 0.0;
	};

	TArray<FWeightedWeather> Candidates;
	TSet<FName> SeenIds;

	double TotalWeight = 0.0;

	for (const auto& Definition : WeatherDefinitions)
	{
		if (!Definition || Definition->WeatherId.IsNone()) continue;
		if (SeenIds.Contains(Definition->WeatherId)) continue;

		SeenIds.Add(Definition->WeatherId);

		const float Weight = Definition->GetWeightForSeason(Season);

		if (!FMath::IsFinite(Weight) || Weight <= 0.0F) continue;

		FWeightedWeather Candidate;
		Candidate.Id = Definition->WeatherId;
		Candidate.Weight = Weight;

		Candidates.Add(Candidate);
		TotalWeight += Weight;
	}

	if (Candidates.IsEmpty()) return NAME_None;

	Candidates.Sort([](const FWeightedWeather& A, const FWeightedWeather& B)
	{
		return A.Id.ToString() < B.Id.ToString();
	});

	const uint64 DayBits = static_cast<uint64>(AbsoluteDay);
	const uint32 DailySeed = static_cast<uint32>(WeatherSeed)
		^ (static_cast<uint32>(DayBits) * 0x9E3779B9u)
		^ (static_cast<uint32>(DayBits >> 32) * 0x85EBCA6Bu);

	FRandomStream Random(static_cast<int32>(DailySeed));
	double Roll = Random.GetFraction() * TotalWeight;

	for (const auto& Candidate : Candidates)
	{
		Roll -= Candidate.Weight;

		if (Roll <= 0.0) return Candidate.Id;
	}

	return Candidates.Last().Id;
}

void USimWeatherComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USimWeatherComponent, WeatherState);
}

void USimWeatherComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (WorldClock)
	{
		WorldClock->OnDayChanged.RemoveDynamic(this, &USimWeatherComponent::HandleDayChanged);
		WorldClock->OnClockRestored.RemoveDynamic(this, &USimWeatherComponent::HandleClockRestored);
	}

	Super::EndPlay(EndPlayReason);
}

void USimWeatherComponent::RefreshWeather()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !WorldClock || !WorldClock->IsClockReady()) return;

	const int64 CurrentDay = FMath::FloorToInt64(WorldClock->GetGameTimeSeconds() / 86400.0);
	const FName TodayWeather = GetWeatherIdForDay(CurrentDay);
	const FName TomorrowWeather = GetWeatherIdForDay(CurrentDay + 1);

	if (TodayWeather.IsNone() || TomorrowWeather.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("Weather: No valid weather definitions."));
		return;
	}

	if (WeatherState.bInitialized && WeatherState.AbsoluteDay == CurrentDay && WeatherState.CurrentWeatherId ==
		TodayWeather && WeatherState.TomorrowWeatherId == TomorrowWeather)
		return;

	const FSimWeatherState PreviousState = WeatherState;

	WeatherState.CurrentWeatherId = TodayWeather;
	WeatherState.TomorrowWeatherId = TomorrowWeather;
	WeatherState.AbsoluteDay = CurrentDay;
	WeatherState.bInitialized = true;

	BroadcastWeatherChanges(PreviousState);

	GetOwner()->ForceNetUpdate();
}

void USimWeatherComponent::BroadcastWeatherChanges(const FSimWeatherState& PreviousState)
{
	if (PreviousState.CurrentWeatherId != WeatherState.CurrentWeatherId)
	{
		OnWeatherChanged.Broadcast(PreviousState.CurrentWeatherId, WeatherState.CurrentWeatherId);
	}
	OnWeatherStateUpdated.Broadcast(WeatherState);
}

void USimWeatherComponent::OnRep_WeatherState(const FSimWeatherState& PreviousState)
{
	BroadcastWeatherChanges(PreviousState);
}
