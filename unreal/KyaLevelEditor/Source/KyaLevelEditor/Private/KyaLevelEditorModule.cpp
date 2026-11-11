#include "Modules/ModuleManager.h"
#include "Editor.h"
#include "HAL/IConsoleManager.h"
#include "LevelEditorViewport.h"

namespace
{
FAutoConsoleCommand ViewportTonemapperCommand(
    TEXT("Kya.ViewportTonemapper"),
    TEXT("Set tone mapping in level editor viewports. Use 0 for native Kya colors or 1 to enable tone mapping."),
    FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
    {
        if (!GEditor || Args.Num() != 1 || (Args[0] != TEXT("0") && Args[0] != TEXT("1"))) return;
        const bool bEnabled = Args[0] == TEXT("1");
        int32 ViewportCount = 0;
        for (FLevelEditorViewportClient* Viewport : GEditor->GetLevelViewportClients())
        {
            if (!Viewport) continue;
            const auto Flag = FEngineShowFlags::EShowFlag::SF_Tonemapper;
            if (Viewport->HandleIsShowFlagEnabled(Flag) != bEnabled)
                Viewport->HandleToggleShowFlag(Flag);
            Viewport->Invalidate();
            ++ViewportCount;
        }
        UE_LOG(LogTemp, Display, TEXT("Kya viewport tone mapping %s in %d level editor viewports"),
            bEnabled ? TEXT("enabled") : TEXT("disabled"), ViewportCount);
    }));
}

IMPLEMENT_MODULE(FDefaultModuleImpl, KyaLevelEditor)
