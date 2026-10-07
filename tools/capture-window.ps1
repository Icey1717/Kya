<#
.SYNOPSIS
Captures the running Kya window, including the debug UI, to a PNG.
.EXAMPLE
& ./tools/capture-window.ps1
.EXAMPLE
& ./tools/capture-window.ps1 -ProcessId 1234 -Screen
#>
[CmdletBinding()]
param(
    [int] $ProcessId,
    [string] $OutputPath,
    [switch] $Screen,
    [switch] $PrintWindow
)

$ErrorActionPreference = 'Stop'
if ($Screen -and $PrintWindow) { throw 'Choose either -Screen or -PrintWindow.' }
if (-not $OutputPath) {
    $OutputPath = Join-Path $PSScriptRoot '../out/screenshots/latest.png'
}
Add-Type -AssemblyName System.Drawing
if (-not ('KyaCapture.Native' -as [type])) {
    Add-Type @'
using System;
using System.Runtime.InteropServices;
namespace KyaCapture {
    public static class Native {
        [StructLayout(LayoutKind.Sequential)]
        public struct Rect { public int Left, Top, Right, Bottom; }
        [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
        [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr window, out Rect rect);
        [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr window);
        [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr window, IntPtr dc, uint flags);
    }
}
'@
}

# Capture at physical pixel resolution on scaled displays.
[void][KyaCapture.Native]::SetProcessDPIAware()
if ($ProcessId) {
    $candidates = @(Get-Process -Id $ProcessId)
} else {
    $candidates = @(Get-Process -Name 'Kya_*' -ErrorAction SilentlyContinue |
        Where-Object { $_.MainWindowHandle -ne 0 })
}
if ($candidates.Count -ne 1) {
    throw "Expected one running Kya window; found $($candidates.Count). Start Kya, or select it with -ProcessId."
}
$app = $candidates[0]
$window = $app.MainWindowHandle
if ($window -eq 0) {
    throw "Process $($app.Id) has no accessible window. Run this script in the same interactive Windows session as Kya."
}
if ([KyaCapture.Native]::IsIconic($window)) {
    throw 'Restore the Kya window before capturing it.'
}
$rect = New-Object KyaCapture.Native+Rect
if (-not [KyaCapture.Native]::GetWindowRect($window, [ref]$rect)) {
    throw 'Could not read the Kya window bounds.'
}
$width = $rect.Right - $rect.Left
$height = $rect.Bottom - $rect.Top
if ($width -le 0 -or $height -le 0) {
    throw "Invalid window dimensions: ${width}x${height}."
}

$OutputPath = [System.IO.Path]::GetFullPath($OutputPath)
[void][System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($OutputPath))
$bitmap = New-Object System.Drawing.Bitmap($width, $height)
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
try {
    if (-not $PrintWindow) {
        # This mode copies visible pixels; other windows must not cover Kya.
        $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
    } else {
        $dc = $graphics.GetHdc()
        try {
            # PW_RENDERFULLCONTENT includes the composed Vulkan window and UI.
            if (-not [KyaCapture.Native]::PrintWindow($window, $dc, 2)) {
                throw 'PrintWindow failed. Retry with -Screen while Kya is fully visible.'
            }
        } finally {
            $graphics.ReleaseHdc($dc)
        }
    }
    $bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
} finally {
    $graphics.Dispose()
    $bitmap.Dispose()
}
Write-Output "Captured $($app.ProcessName) (PID $($app.Id)), ${width}x${height}: $OutputPath"
