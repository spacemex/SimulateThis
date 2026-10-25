// Fill out your copyright notice in the Description page of Project Settings.

// ReSharper disable CppOverrideWithDifferentVisibility
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"

#include "SimCalendarEventComponent.generated.h"


class USimWorldClockComponent;
class USimCalendarEventData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSimCalendarEventChanged, FName, EventId);

UCLASS(ClassGroup=(Simulation), meta=(BlueprintSpawnableComponent))
class SIMULATETHIS_API USimCalendarEventComponent : public UActorComponent
{
private:
	GENERATED_BODY()

public:
	USimCalendarEventComponent();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Simulation|Events")
	TArray<TObjectPtr<USimCalendarEventData>> EventDefinitions;

	UPROPERTY(BlueprintAssignable, Category="Sumulation|Events")
	FSimCalendarEventChanged OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category="Sumulation|Events")
	FSimCalendarEventChanged OnEventEnded;

	UFUNCTION(BlueprintPure, Category="Simulation|Events")
	TArray<FName> GetActiveEventIds() const;

	UFUNCTION(BlueprintPure, Category="Simulation|Events")
	bool IsEventActive(FName EventId) const;

	UFUNCTION(BlueprintPure, Category="Simulation|Events")
	USimCalendarEventData* FindEventDefinition(FName EventId) const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_ActiveEventIds)
	TArray<FName> ActiveEventIds;

	UPROPERTY(Transient)
	TObjectPtr<USimWorldClockComponent> WorldClock;

	FTimerHandle InitializationTimer;

	void TryInitialize();
	void EvaluateActiveEvents();

	void BroadcastChanges(const TArray<FName>& Previous, const TArray<FName>& Current);

	UFUNCTION()
	void HandleDayChanged(int32 NewDay);
	
	UFUNCTION()
	void HandleClockRestored();

	UFUNCTION()
	void OnRep_ActiveEventIds(const TArray<FName>& PreviousEventIds);
};
