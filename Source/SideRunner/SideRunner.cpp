// Fill out your copyright notice in the Description page of Project Settings.

#include "SideRunner.h"
#include "Modules/ModuleManager.h"

// Define custom log categories
DEFINE_LOG_CATEGORY(LogSideRunner);
DEFINE_LOG_CATEGORY(LogSideRunnerScoring);
DEFINE_LOG_CATEGORY(LogSideRunnerCombat);

IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, SideRunner, "SideRunner" );
