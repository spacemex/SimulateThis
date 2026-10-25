#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "SimWorldSaveGame.generated.h"


USTRUCT(BlueprintType)
struct SIMULATETHIS_API FSimClockSaveData
{
	GENERATED_BODY()

	UPROPERTY()
	double GameSeconds = 0.0;

	UPROPERTY()
	float TimeScale = 60.0F;

	UPROPERTY()
	int32 StartingYear = 1;

	UPROPERTY()
	int32 DaysPerSeason = 14;
};

UCLASS()
class SIMULATETHIS_API USimWorldSaveGame : public USaveGame
{
private:
	GENERATED_BODY()

public:
	UPROPERTY()
	int32 SaveVersion = 1;

	UPROPERTY()
	FSimClockSaveData ClockData;
};
