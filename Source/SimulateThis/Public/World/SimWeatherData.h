#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "World/Types/SimCalendarTypes.h"

#include "SimWeatherData.generated.h"

UCLASS(BlueprintType)
class SIMULATETHIS_API USimWeatherData : public UPrimaryDataAsset
{
private:
	GENERATED_BODY()

public:
	/**
	 * The unique Identifier for this weather event
	 * <example>
	 * Weathers.Rain
	 * </example>
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FName WeatherId = NAME_None;

	/**
	 * The Translatable DisplayName of this weather event
	 * <example>
	 * weather.rain = Rain
	 * </example>
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FText DisplayName;

	/**
	 * The Tags that are associated with this weather event
	 * <example>
	 * Demands.Baking, Debuffs.Gloomy 
	 * </example>
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FGameplayTagContainer WeatherTags;

	/**
	 * The Weight For this weather event in Spring
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float SpringWeight = 1.0F;

	/**
	 * The Weight For this weather event in Summer
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float SummerWeight = 1.0F;

	/**
	 * The Weight For this weather event in Autumn
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float AutumnWeight = 1.0F;

	/**
	 * The Weight For this weather event in Winter
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float WinterWeight = 1.0F;

	/**
	 * Min And Max possible temperature range in Celsius
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather|Conditions")
	FVector2D TemperatureCelsiusRange = FVector2D(10.0, 25.0);

	/**
	 * Relative humidity, from 0% to 100%
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather|Conditions")
	FVector2D HumidityPercentRange = FVector2D(40.0, 75.0);

	/**
	 * Wind speed in kilometers per hour
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather|Conditions")
	FVector2D WindSpeedKmhRange = FVector2D(0.0, 15.0);

	/**
	 * Cloud coverage, from 0.0 to 1.0
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather|Conditions")
	FVector2D CloudCoverageRange = FVector2D(0.0, 0.25);

	/**
	 * Precipitation strength, from 0.0 to 1.0
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather|Conditions")
	FVector2D PrecipitationIntensityRange = FVector2D(0.0, 0.0);

	FORCEINLINE float GetWeightForSeason(ESimSeason Season) const
	{
		switch (Season)
		{
		case ESimSeason::Spring: return SpringWeight;
		case ESimSeason::Summer: return SummerWeight;
		case ESimSeason::Autumn: return AutumnWeight;
		case ESimSeason::Winter: return WinterWeight;
		default: return 0.0F;
		}
	}
};
