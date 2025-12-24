# Run the built executable with administrator privileges

$exePath = ".\build\windows\x64\release\escModKey.exe"

if (-not (Test-Path $exePath)) {
    Write-Host "Error: Executable not found at $exePath" -ForegroundColor Red
    Write-Host "Please build the project first using: xmake" -ForegroundColor Yellow
    exit 1
}

Write-Host "Running $exePath with administrator privileges..." -ForegroundColor Green

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if ($isAdmin) {
    # Already running as admin, just run the exe
    & $exePath
} else {
    # Not admin, restart with admin privileges
    Write-Host "Requesting administrator privileges..." -ForegroundColor Yellow
    Start-Process -FilePath $exePath -Verb RunAs -Wait
}
