// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "GASDebuggerStyle.h"

class FGASDebuggerCommands : public TCommands<FGASDebuggerCommands>
{
public:
	FGASDebuggerCommands()
		: TCommands<FGASDebuggerCommands>(
			TEXT("GASDebugger"),
			NSLOCTEXT("Contexts", "GASDebugger", "GAS Debugger Plugin"),
			NAME_None,
			FGASDebuggerStyle::GetStyleSetName())
	{
	}

	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenGASDebugger;
	TSharedPtr<FUICommandInfo> OpenGASDebugger_Button;
};
