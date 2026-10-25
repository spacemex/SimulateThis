#pragma once

#include "CoreMinimal.h"
#include "SimWeatherTypes.generated.h"

USTRUCT(BlueprintType)
struct SIMULATETHIS_API FSimWeatherConditions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TemperatureCelsius = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HumidityPercent = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float WindSpeedKmh = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CloudCoverage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PrecipitationIntensity = 0.0f;
};
