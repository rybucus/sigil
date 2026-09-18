@echo off
setlocal
if exist "%~dp0local.cmd" call "%~dp0local.cmd"
set "MODE=-i"
if /i "%~1"=="--check" set "MODE=--dry-run --Werror"
if not defined SIGIL_CLANG_FORMAT (
  if defined VCINSTALLDIR set "SIGIL_CLANG_FORMAT=%VCINSTALLDIR%Tools\Llvm\x64\bin\clang-format.exe"
)
if not defined SIGIL_CLANG_FORMAT (
  if defined SIGIL_VSDEVCMD (
    for %%i in ("%SIGIL_VSDEVCMD%\..\..\..") do set "SIGIL_CLANG_FORMAT=%%~fi\VC\Tools\Llvm\x64\bin\clang-format.exe"
  )
)
if not defined SIGIL_CLANG_FORMAT set "SIGIL_CLANG_FORMAT=clang-format"
set FAILED=0
call :run "%~dp0..\include"
call :run "%~dp0..\tests"
if "%FAILED%"=="1" (
  echo tools\format.cmd: formatting differences found. 1>&2
  exit /b 1
)
exit /b 0

:run
for /r "%~1" %%f in (*.cxx *.hxx) do (
  "%SIGIL_CLANG_FORMAT%" %MODE% "%%f"
  if errorlevel 1 set FAILED=1
)
exit /b 0
