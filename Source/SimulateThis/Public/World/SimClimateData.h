#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "World/Types//SimCalendarTypes.h"
#include "SimClimateData.generated.h"

USTRUCT(BlueprintType)
struct SIMULATETHIS_API FSimSeasonClimateSettings
{
	GENERATED_BODY()

	/**
	 * Added to the weather's generated temperature
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float TemperatureOffsetCelsius = 0.0f;

	/**
	 * Percentage points added to the humidity
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HumidityOffsetPercent = 0.0f;

	/**
	 * Multiplies the weather's generated wind speed
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float WindMultiplier = 1.0f;
};

UCLASS(BlueprintType)
class SIMULATETHIS_API USimClimateData : public UPrimaryDataAsset
{
private:
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate")
	FName ClimateId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate")
	FText DisplayName;

	/**
	 * Optional multiplier for each weather type

	 * 
	 * Missing entries use 1.0
	 * 
	 * Zero disables that weather in this climate
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate|Weather")
	TMap<FName, float> WeatherWeightMultipliers;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate|Seasons")
	FSimSeasonClimateSettings Spring;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate|Seasons")
	FSimSeasonClimateSettings Summer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate|Seasons")
	FSimSeasonClimateSettings Autumn;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Climate|Seasons")
	FSimSeasonClimateSettings Winter;

	const FSimSeasonClimateSettings& GetSeasonSettings(ESimSeason Season) const
	{
		switch (Season)
		{
		case ESimSeason::Summer: return Summer;
		case ESimSeason::Autumn: return Autumn;
		case ESimSeason::Winter: return Winter;
		default: return Spring;
		}
	}

	float GetWeatherMultiplier(FName WeatherId) const
	{
		const float* Multiplier = WeatherWeightMultipliers.Find(WeatherId);

		if (!Multiplier) return 1.0f;

		return FMath::IsFinite(*Multiplier) ? FMath::Max(0.0f, *Multiplier) : 0.0f;
	}
};
