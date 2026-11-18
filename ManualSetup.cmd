@echo off
setlocal DisableDelayedExpansion
rem SPDX-License-Identifier: MIT
set "SM_MANUAL_SCRIPT=%~dp0Supermoon\tools\ManualSetup.ps1"
if not exist "%SM_MANUAL_SCRIPT%" set "SM_MANUAL_SCRIPT=%~dp0tools\ManualSetup.ps1"
if not exist "%SM_MANUAL_SCRIPT%" goto missing
echo Supermoon manual preparation uses Windows PowerShell 5.1.
echo It does not install files into the game. Existing Windows restrictions remain in force.
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoLogo -NoProfile -ExecutionPolicy RemoteSigned -File "%SM_MANUAL_SCRIPT%" -Interactive
if errorlevel 1 goto failed
pause
exit /b 0
:failed
echo Manual preparation or verification failed. Keep NMS closed and read the reported reason.
echo Read SETUP_SECURITY.txt if Windows blocked the tool. No persistent policy was changed.
pause
exit /b 1
:missing
echo Extract the complete package before running ManualSetup.cmd.
pause
exit /b 1
