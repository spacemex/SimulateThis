// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SimGameModeBase.h"

#include "Core/SimGameStateBase.h"
#include "Core/SimPlayerController.h"
#include "Core/SimPlayerState.h"

ASimGameModeBase::ASimGameModeBase()
{
	PlayerControllerClass = ASimPlayerController::StaticClass();
	GameStateClass = ASimGameStateBase::StaticClass();
	PlayerStateClass = ASimPlayerState::StaticClass();
}
