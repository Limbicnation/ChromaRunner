// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Custom log categories for ChromaRunner
// Use these instead of LogTemp for better log management and performance
// By default, only Warning level and above will be logged

/** General gameplay logging - errors and warnings only by default */
DECLARE_LOG_CATEGORY_EXTERN(LogSideRunner, Log, All);

/** Scoring system logging - warnings only by default to reduce spam */
DECLARE_LOG_CATEGORY_EXTERN(LogSideRunnerScoring, Warning, All);

/** Combat and damage logging - warnings only by default */
DECLARE_LOG_CATEGORY_EXTERN(LogSideRunnerCombat, Warning, All);

