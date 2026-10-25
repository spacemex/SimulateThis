// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppOverrideWithDifferentVisibility
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "World/SimCalendarTypes.h"
#include "SimWorldClockComponent.generated.h"

USTRUCT()
struct SIMULATETHIS_API FSimClockState
{
	GENERATED_BODY()

	UPROPERTY()
	double GameSecondsAtAnchor = 0.0;

	UPROPERTY()
	double ServerSecondsAtAnchor = 0.0;

	UPROPERTY()
	float TimeScale = 60.0f;

	UPROPERTY()
	int32 StartingYear = 1;

	UPROPERTY()
	int32 DaysPerSeason = 14;

	UPROPERTY()
	bool bInitialized = false;

	UPROPERTY()
	int32 Revision = 0;
};

struct FSimClockSaveData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimHourChanged, int32, NewHour);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimDayChanaged, int32, NewDay);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSimSeasonChanged, ESimSeason, NewSeason, int32, CurrentYear);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimYearChanged, int32, CurrentYear);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSimClockRestored);

UCLASS(ClassGroup=(Simulation))
class SIMULATETHIS_API USimWorldClockComponent : public UActorComponent
{
private:
	GENERATED_BODY()

public:
	USimWorldClockComponent();

	UPROPERTY(BlueprintAssignable, Category="Simulation|Time")
	FSimHourChanged OnHourChanged;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Time")
	FSimDayChanaged OnDayChanged;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Calendar")
	FSimSeasonChanged OnSeasonChanged;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Calendar")
	FSimYearChanged OnYearChanged;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Presistence")
	FSimClockRestored OnClockRestored;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Time", meta=(ClampMin="0", ClampMax="23"))
	int32 StartingHour = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Time", meta=(ClampMin="0.0"))
	float InitialTimeScale = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Time", meta=(ClampMin="1"))
	int32 InitialYear = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Time", meta=(ClampMin="1"))
	int32 SeasonLengthInDays = 14;

	UFUNCTION(BlueprintPure, Category="Simulation|Time")
	bool IsClockReady() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Time")
	double GetGameTimeSeconds() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Time")
	int32 GetDay() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Time")
	int32 GetHour() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Time")
	int32 GetMinute() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Calendar")
	FSimCalendarDate GetCalendarDate() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Calendar")
	int32 GetDaysPerSeason() const;

	UFUNCTION(BlueprintCallable, Category="Simulation|Time")
	void SetTimeScale(float NewTimeScale);

	FSimClockSaveData CreateSaveData() const;
	bool RestoreFromSaveData(const FSimClockSaveData& SaveData);

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_ClockState)
	FSimClockState ClockState;


	UFUNCTION()
	void OnRep_ClockState(const FSimClockState& PreviousState);

	double GetServerTimeSeconds() const;
	void CheckCalendarEvents();
	FTimerHandle CalendarPollHandle;
	int64 LastObservedHour = -1;
	int64 LastObservedDay = -1;
	bool bHasClockSample = false;
	FSimCalendarDate BuildCalendarDate(int64 AbsoluteDay) const;
};
