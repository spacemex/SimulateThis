#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "World/SimCalendarTypes.h"

#include "SimWeatherData.generated.h"

UCLASS(BlueprintType)
class SIMULATETHIS_API USimWeatherData : public UPrimaryDataAsset
{
private:
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FName WeatherId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather")
	FGameplayTagContainer WeatherTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float SprintWeight = 1.0F;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float SummerWeight = 1.0F;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float AutumnWeight = 1.0F;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Weather", meta=(ClampMin="0.0"))
	float WinterWeight = 1.0F;

	FORCEINLINE float GetWeightForSeason(ESimSeason Season) const
	{
		switch (Season)
		{
		case ESimSeason::Spring: return SprintWeight;
		case ESimSeason::Summer: return SummerWeight;
		case ESimSeason::Autumn: return AutumnWeight;
		case ESimSeason::Winter: return WinterWeight;
		default: return 0.0F;
		}
	}
};
