# Run the virtual key detector test

$exePath = ".\build\windows\x64\release\test_virtual_detector.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "Error: Test executable not found at $exePath" -ForegroundColor Red
    Write-Host "Please build the project first using: xmake" -ForegroundColor Yellow
    exit 1
}

Write-Host "Running Virtual Key Detector Test..." -ForegroundColor Green
& $exePath
