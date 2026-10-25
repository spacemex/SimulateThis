#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "SimCalendarTypes.h"

#include "SimCalendarEventData.generated.h"

UCLASS(BlueprintType)
class SIMULATETHIS_API USimCalendarEventData : public UPrimaryDataAsset
{
private:
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event")
	FName EventId = NAME_Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event")
	ESimSeason StartSeason = ESimSeason::Spring;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event", meta=(ClampMin="1"))
	int32 StartDayOfSeason = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event", meta=(ClampMin="1"))
	int32 DurationDays = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event")
	bool bEnabled = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Calendar Event")
	FGameplayTagContainer EventTags;
};
