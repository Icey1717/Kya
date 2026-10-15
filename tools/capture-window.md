# Capturing the running app

From the repository root, with Kya running:

```powershell
& ./tools/capture-window.ps1
```

The PNG is written to `out/screenshots/latest.png` (ignored by Git). It includes
the main window's game viewport and debug UI. Detached ImGui windows are separate
windows and are not included. Capture does not send input or change game state.

If multiple Kya builds are running, use `-ProcessId <pid>`. Use
`-OutputPath out/screenshots/before.png` to retain a named capture.

The default passively copies the window's screen rectangle (also available as
`-Screen`). Make Kya fully visible; overlapping windows appear in the capture.
Use `-PrintWindow` only when necessary: it requests a window repaint and can
cause visible flashing or blank images with Vulkan. Restore minimized windows first.
Run in the same interactive Windows session as the app.

Enable **Render When Minimized** under the debug **Rendering > Pipeline** controls
to keep updating the game's Vulkan framebuffer while the window is minimized.
This setting is saved in `settings.json` and defaults to off. Game simulation
continues; the window's debug UI is not rendered until restored. Rendering keeps
the existing render-buffer resolution and waits briefly for window events each
frame instead of relying on presentation/VSync for pacing.

This supports GPU screenshot readback through `Debug::SaveLoad::CaptureScreenshot()`;
the desktop-copy script above still requires a visible window. For full-resolution
PNG captures over localhost, enable the separate [debug socket](debug_socket/README.md).

The standalone Vulkan regression test exercises minimize/restore cycles and checks
fresh screenshot pixels while minimized:

```powershell
Push-Location bin/WIN
./KyaPortTest.exe --gtest_filter=MinimizedRendering.* --gtest_also_run_disabled_tests
Pop-Location
```

Run this test separately from the normal test suite; it creates a window and requires
an interactive desktop.

An agent can run this command and inspect the resulting PNG using `view_image`;
there is no need to attach screenshots manually.
