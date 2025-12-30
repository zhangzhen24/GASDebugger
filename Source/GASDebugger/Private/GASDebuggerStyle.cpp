// Copyright Epic Games, Inc. All Rights Reserved.

#include "GASDebuggerStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FGASDebuggerStyle::StyleInstance = nullptr;

void FGASDebuggerStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FGASDebuggerStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FGASDebuggerStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("GASDebuggerStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef<FSlateStyleSet> FGASDebuggerStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("GASDebuggerStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("GASDebugger")->GetBaseDir() / TEXT("Resources"));

	Style->Set("GASDebugger.OpenGASDebugger", new IMAGE_BRUSH(TEXT("ButtonIcon_40x"), Icon16x16));
	Style->Set("GASDebugger.OpenGASDebugger_Button", new IMAGE_BRUSH(TEXT("Icon128"), Icon20x20));

	return Style;
}

#undef IMAGE_BRUSH

void FGASDebuggerStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FGASDebuggerStyle::Get()
{
	return *StyleInstance;
}
