// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SimWeatherComponent.generated.h"

class USimWeatherData;
class USimWorldClockComponent;

USTRUCT(BlueprintType)
struct SIMULATETHIS_API FSimWeatherState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FName CurrentWeatherId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	FName TomorrowWeatherId = NAME_None;

	UPROPERTY(BlueprintReadOnly)
	int64 AbsoluteDay = -1;

	UPROPERTY(BlueprintReadOnly)
	bool bInitialized = false;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FSimWeatherChanged, FName, PreviousWeather, FName, NewWeather);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimWeatherStateUpdated, FSimWeatherState, NewState);

UCLASS(ClassGroup=(Simulation))
class SIMULATETHIS_API USimWeatherComponent : public UActorComponent
{
private:
	GENERATED_BODY()

public:
	USimWeatherComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	TArray<TObjectPtr<USimWeatherData>> WeatherDefinitions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	int32 WeatherSeed = 12345;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Weather")
	FSimWeatherChanged OnWeatherChanged;

	UPROPERTY(BlueprintAssignable, Category="Simulation|Weather")
	FSimWeatherStateUpdated OnWeatherStateUpdated;

	UFUNCTION(BlueprintPure, Category="Simulation|Weather")
	FSimWeatherState GetWeatherState() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Weather")
	FName GetCurrentWeatherId() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Weather")
	FName GetTomorrowWeatherId() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Weather")
	USimWeatherData* FindWeatherDefinition(FName WeatherId) const;

	void InitializeWeather();

	FName GetWeatherIdForDay(int64 AbsoluteDay) const;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	UPROPERTY(ReplicatedUsing=OnRep_WeatherState)
	FSimWeatherState WeatherState;

	UPROPERTY(Transient)
	TObjectPtr<USimWorldClockComponent> WorldClock;

	void RefreshWeather();

	void BroadcastWeatherChanges(const FSimWeatherState& PreviousState);

	UFUNCTION()
	void HandleDayChanged(int32 NewDay);

	UFUNCTION()
	void HandleClockRestored();

	UFUNCTION()
	void OnRep_WeatherState(const FSimWeatherState& PreviousState);
};
