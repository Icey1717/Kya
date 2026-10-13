param([switch]$DownloadSpan)
$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$modelRoot = Join-Path $repoRoot 'upscaling/models'
New-Item -ItemType Directory -Force $modelRoot | Out-Null
$spanPath = Join-Path $modelRoot '4xNomosUni_span_multijpg_fp32_opset17.onnx'
if (!(Test-Path -LiteralPath $spanPath)) {
    if (!$DownloadSpan) { throw 'SPAN ONNX is missing. Re-run with -DownloadSpan to fetch the author export.' }
    Invoke-WebRequest -Uri 'https://github.com/Phhofm/models/releases/download/4xNomosUni_span_multijpg/4xNomosUni_span_multijpg_fp32_opset17.onnx' -OutFile $spanPath
}
$spanHash = (Get-FileHash -LiteralPath $spanPath -Algorithm SHA256).Hash.ToLowerInvariant()
if ($spanHash -ne 'a435b009109e72c50ce95927dab0a6dde63e594cf57ba5a18ba63da67355698a') {
    throw 'SPAN ONNX SHA256 mismatch'
}
if (!(Test-Path -LiteralPath (Join-Path $modelRoot '4x-UltraSharpV2_fp32_op17.onnx'))) {
    throw 'The existing UltraSharp ONNX file must be supplied in upscaling/models.'
}
Write-Host "In-game models ready: $modelRoot"
