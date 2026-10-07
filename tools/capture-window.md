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

An agent can run this command and inspect the resulting PNG using `view_image`;
there is no need to attach screenshots manually.
