#!/usr/bin/env pwsh

Write-Host "[BUILD] Generating Visual Studio 2022 solution..." -ForegroundColor Green

# Generate the Visual Studio solution
xmake project -k vsxmake2022

if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] Solution generated successfully!" -ForegroundColor Green
    Write-Host "[INFO] Solution file: vsxmake2022/cpp_mods.sln" -ForegroundColor Cyan
    
    # Ask if user wants to open the solution
    $openSln = Read-Host "Open Visual Studio now? (y/n)"
    if ($openSln -eq "y" -or $openSln -eq "Y") {
        if (Test-Path "vsxmake2022/cpp_mods.sln") {
            Write-Host "[LAUNCH] Opening Visual Studio..." -ForegroundColor Blue
            Start-Process "vsxmake2022/cpp_mods.sln"
        } else {
            Write-Host "[ERROR] Solution file not found!" -ForegroundColor Red
        }
    }
} else {
    Write-Host "[ERROR] Failed to generate solution!" -ForegroundColor Red
    Write-Host "[TIP] Make sure xmake is installed and run this from project root" -ForegroundColor Yellow
}

Write-Host "Press any key to continue..." -ForegroundColor Gray
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown") 