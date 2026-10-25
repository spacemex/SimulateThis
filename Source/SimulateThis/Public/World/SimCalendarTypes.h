#pragma once

#include "CoreMinimal.h"
#include "SimCalendarTypes.generated.h"

UENUM(BlueprintType)
enum class ESimSeason : uint8
{
	Spring UMETA(DisplayName = "Spring"),
	Summer UMETA(DisplayName = "Summer"),
	Autumn UMETA(DisplayName = "Autumn"),
	Winter UMETA(DisplayName = "Winter")
};

USTRUCT(BlueprintType)
struct SIMULATETHIS_API FSimCalendarDate
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Calendar")
	int32 Year = 1;

	UPROPERTY(BlueprintReadOnly, Category="Calendar")
	int32 DayOfYear = 1;

	UPROPERTY(BlueprintReadOnly, Category="Calendar")
	int32 DayOfSeason = 1;

	UPROPERTY(BlueprintReadOnly, Category="Calendar")
	ESimSeason Season = ESimSeason::Spring;

	UPROPERTY(BlueprintReadOnly, Category="Calendar")
	bool bIsValid = false;
};
