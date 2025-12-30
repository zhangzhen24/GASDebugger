// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASDebuggerCommands.h"

#define LOCTEXT_NAMESPACE "FGASDebuggerModule"

void FGASDebuggerCommands::RegisterCommands()
{
	UI_COMMAND(OpenGASDebugger, "GAS Debugger", "Open the GAS Debugger window", EUserInterfaceActionType::Check, FInputChord());
	UI_COMMAND(OpenGASDebugger_Button, "GAS Debugger", "Open the GAS Debugger window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
