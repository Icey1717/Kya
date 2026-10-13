param(
    [string]$KyaBankExe = 'G:\repos\KyaBank\out\build\x64-debug\KyaBank.exe',
    [string]$Bank = 'assets/CDEURO/LEVEL/LEVEL_1/LEVEL.BNK',
    [switch]$DownloadModels
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$outputRoot = Join-Path $repoRoot 'out/upscale-compare'
$bankPath = if ([IO.Path]::IsPathRooted($Bank)) { $Bank } else { Join-Path $repoRoot $Bank }
if (!(Test-Path -LiteralPath $KyaBankExe -PathType Leaf)) { throw "KyaBank not found: $KyaBankExe" }
if (!(Test-Path -LiteralPath $bankPath -PathType Leaf)) { throw "Bank not found: $bankPath" }
foreach ($folder in @('extracted', 'textures', 'selected', 'models')) {
    New-Item -ItemType Directory -Force (Join-Path $outputRoot $folder) | Out-Null
}

& $KyaBankExe extract $bankPath --output (Join-Path $outputRoot 'extracted') --flatten *> (Join-Path $outputRoot 'extract.log')
if ($LASTEXITCODE -ne 0) { throw 'Bank extraction failed; see out/upscale-compare/extract.log' }
& $KyaBankExe texconvert (Join-Path $outputRoot 'extracted') --output (Join-Path $outputRoot 'textures') *> (Join-Path $outputRoot 'texconvert.log')
if ($LASTEXITCODE -ne 0) { throw 'Texture conversion failed; see out/upscale-compare/texconvert.log' }

# Original game textures: character atlas, UI portrait, painted surface, foliage, prop, character.
$samples = @(
    'KIM_Scene01_L.O.D._M18_L0.png',
    'CMP_ICONS_M0_L0.png',
    'ACTORSCOMMONMATERIAL_M40_L0.png',
    'BACK_SECT2_Scene01_Background_sect02_M5_L0.png',
    'BOX_01_LOW_Scene01_box_01_low_M0_L0.png',
    'ATON_Scene01_Aton_M0_L0.png'
)
$exported = @()
foreach ($name in $samples) {
    $sourcePath = Join-Path (Join-Path $outputRoot 'textures') $name
    if (!(Test-Path -LiteralPath $sourcePath)) { throw "Expected Level 1 sample missing: $name" }
    Copy-Item -LiteralPath $sourcePath -Destination (Join-Path $outputRoot 'selected')
    $exported += [ordered]@{ filename = $name; sha256 = (Get-FileHash -LiteralPath $sourcePath -Algorithm SHA256).Hash.ToLowerInvariant() }
}
[ordered]@{
    bank = $bankPath
    bank_sha256 = (Get-FileHash -LiteralPath $bankPath -Algorithm SHA256).Hash.ToLowerInvariant()
    exporter = (Resolve-Path -LiteralPath $KyaBankExe).Path
    exporter_sha256 = (Get-FileHash -LiteralPath $KyaBankExe -Algorithm SHA256).Hash.ToLowerInvariant()
    textures = $exported
} | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $outputRoot 'texture_sources.json') -Encoding utf8

if ($DownloadModels) {
    $sources = Get-Content -LiteralPath (Join-Path $PSScriptRoot 'model_sources.json') -Raw | ConvertFrom-Json
    foreach ($property in $sources.models.PSObject.Properties) {
        $model = $property.Value
        $targetPath = Join-Path (Join-Path $outputRoot 'models') $model.filename
        if (!(Test-Path -LiteralPath $targetPath)) {
            Write-Host "Downloading $($model.filename)"
            Invoke-WebRequest -Uri $model.url -OutFile $targetPath
        }
        $actualHash = (Get-FileHash -LiteralPath $targetPath -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actualHash -ne $model.sha256) { throw "SHA256 mismatch for $($model.filename)" }
    }
}
Write-Host "Original game textures ready: $outputRoot/selected"
