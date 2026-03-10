// Forces all self-registering debug module TUs to be included by the linker.
// Each extern declaration references a named symbol in a module's .cpp, which
// prevents the linker from discarding that object file from the static library.
// The actual registration happens in the static initializers of each TU,
// before main() is called.
#include "DebugMenu.h"

namespace Debug {
    extern MenuRegisterer sDebugCollisionMenuReg;
    extern UpdateRegisterer sDebugCollisionUpdateReg;

    extern MenuRegisterer sDebugTextureMenuReg;
    extern UpdateRegisterer sDebugTextureUpdateReg;

    extern MenuRegisterer sDebugShopMenuReg;
    extern UpdateRegisterer sDebugShopUpdateReg;

    extern MenuRegisterer sDebugScenarioMenuReg;
    extern UpdateRegisterer sDebugScenarioUpdateReg;

    extern MenuRegisterer sDebugSaveLoadMenuReg;
    extern UpdateRegisterer sDebugSaveLoadUpdateReg;

    extern MenuRegisterer sDebugHeroReplayMenuReg;
    extern UpdateRegisterer sDebugHeroReplayUpdateReg;

    extern MenuRegisterer sDebugSceneMenuReg;
    extern StartupRegisterer sDebugSceneStartupReg;

    extern MenuRegisterer sDebugRenderingMenuReg;
    extern StartupRegisterer sDebugRenderingStartupReg;

    extern MenuRegisterer sDebugLogMenuReg;

    extern MenuRegisterer sDebugFramebufferMenuReg;
    extern MenuRegisterer sDebugFramebuffersMenuReg;

    extern MenuRegisterer sDebugCinematicMenuReg;
    extern MenuRegisterer sDebugHeroMenuReg;
    extern MenuRegisterer sDebugMemoryMenuReg;
    extern MenuRegisterer sDebugMeshMenuReg;
    extern MenuRegisterer sDebugCheckpointMenuReg;
    extern MenuRegisterer sDebugEventMenuReg;
    extern MenuRegisterer sDebugWolfenMenuReg;
    extern MenuRegisterer sDebugTutorialMenuReg;
    extern MenuRegisterer sDebugInputMenuReg;
    extern MenuRegisterer sDebugActorMenuReg;

    void EnsureRegistrations()
    {
        (void)&sDebugCollisionMenuReg;
        (void)&sDebugCollisionUpdateReg;
        (void)&sDebugTextureMenuReg;
        (void)&sDebugTextureUpdateReg;
        (void)&sDebugShopMenuReg;
        (void)&sDebugShopUpdateReg;
        (void)&sDebugScenarioMenuReg;
        (void)&sDebugScenarioUpdateReg;
        (void)&sDebugSaveLoadMenuReg;
        (void)&sDebugSaveLoadUpdateReg;
        (void)&sDebugHeroReplayMenuReg;
        (void)&sDebugHeroReplayUpdateReg;
        (void)&sDebugSceneMenuReg;
        (void)&sDebugSceneStartupReg;
        (void)&sDebugRenderingMenuReg;
        (void)&sDebugRenderingStartupReg;
        (void)&sDebugLogMenuReg;
        (void)&sDebugFramebufferMenuReg;
        (void)&sDebugFramebuffersMenuReg;
        (void)&sDebugCinematicMenuReg;
        (void)&sDebugHeroMenuReg;
        (void)&sDebugMemoryMenuReg;
        (void)&sDebugMeshMenuReg;
        (void)&sDebugCheckpointMenuReg;
        (void)&sDebugEventMenuReg;
        (void)&sDebugWolfenMenuReg;
        (void)&sDebugTutorialMenuReg;
        (void)&sDebugInputMenuReg;
        (void)&sDebugActorMenuReg;
    }
}