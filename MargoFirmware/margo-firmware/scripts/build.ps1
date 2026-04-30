param(
    [string]$Python = "$env:USERPROFILE\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe"
)

$ErrorActionPreference = "Stop"
$ProjectRoot = Resolve-Path (Join-Path $PSScriptRoot "..")
$BuildDir = Join-Path $ProjectRoot ".pio\build\m5stack-sticks3"
$DistDir = Join-Path $ProjectRoot "dist"
$Esptool = Join-Path $env:USERPROFILE ".platformio\packages\tool-esptoolpy\esptool.py"
$BootApp0 = Join-Path $env:USERPROFILE ".platformio\packages\framework-arduinoespressif32\tools\partitions\boot_app0.bin"
$MergedBin = Join-Path $DistDir "Margo-m5stack-sticks3.bin"

if (!(Test-Path $Python)) {
    throw "Python not found: $Python"
}

Push-Location $ProjectRoot
try {
    & $Python -m platformio run
    New-Item -ItemType Directory -Force $DistDir | Out-Null
    & $Python $Esptool --chip esp32s3 merge_bin `
        -o $MergedBin `
        --flash_mode dio `
        --flash_freq 80m `
        --flash_size 8MB `
        0x0000 (Join-Path $BuildDir "bootloader.bin") `
        0x8000 (Join-Path $BuildDir "partitions.bin") `
        0xe000 $BootApp0 `
        0x10000 (Join-Path $BuildDir "firmware.bin")
    Write-Host "Built $MergedBin"
}
finally {
    Pop-Location
}
