@echo off
setlocal
if exist "%~dp0local.cmd" call "%~dp0local.cmd"
if defined SIGIL_VSDEVCMD goto :found
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
  for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "SIGIL_VSDEVCMD=%%i\Common7\Tools\VsDevCmd.bat"
)
if defined SIGIL_VSDEVCMD goto :found
echo tools\build.cmd: Visual Studio not found. Set SIGIL_VSDEVCMD to the path of VsDevCmd.bat, or create tools\local.cmd that sets it. 1>&2
exit /b 1
:found
if not exist "%SIGIL_VSDEVCMD%" (
  echo tools\build.cmd: "%SIGIL_VSDEVCMD%" does not exist. 1>&2
  exit /b 1
)
call "%SIGIL_VSDEVCMD%" -arch=x64 -host_arch=x64 -no_logo
if errorlevel 1 exit /b 1
cd /d "%~dp0.."
%*
