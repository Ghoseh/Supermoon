@echo off
setlocal DisableDelayedExpansion
rem SPDX-License-Identifier: MIT
rem No forwarded command-line text, downloads, elevation, or persistent policy changes.
set "SM_SETUP_SCRIPT=%~dp0Supermoon\tools\Setup.ps1"
if not exist "%SM_SETUP_SCRIPT%" set "SM_SETUP_SCRIPT=%~dp0tools\Setup.ps1"
if not exist "%SM_SETUP_SCRIPT%" goto missing
echo Supermoon setup uses Windows PowerShell 5.1.
echo RemoteSigned applies only to this process. Windows/organization restrictions remain in force.
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy RemoteSigned -File "%SM_SETUP_SCRIPT%" -Interactive
if errorlevel 1 goto failed
echo.
pause
exit /b 0
:failed
echo Setup failed or was blocked. Read SETUP_SECURITY.txt. No persistent execution policy was changed.
pause
exit /b 1
:missing
echo Setup files are missing. Extract the complete archive before running Setup.cmd.
pause
exit /b 1
