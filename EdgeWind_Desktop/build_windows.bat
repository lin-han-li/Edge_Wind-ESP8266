@echo off
setlocal

cd /d %~dp0

rem Ensure no running instance blocks overwrite
taskkill /F /IM EdgeWind_Monitor.exe >nul 2>nul

rem Output root on D:\
set OUTPUT_ROOT=D:\Edge_Wind
if not exist "%OUTPUT_ROOT%" mkdir "%OUTPUT_ROOT%" >nul 2>nul
if not exist "%OUTPUT_ROOT%\dist" mkdir "%OUTPUT_ROOT%\dist" >nul 2>nul
if not exist "%OUTPUT_ROOT%\build" mkdir "%OUTPUT_ROOT%\build" >nul 2>nul
if not exist "%OUTPUT_ROOT%\installer" mkdir "%OUTPUT_ROOT%\installer" >nul 2>nul

set PROJECT_ROOT=%~dp0..
set EDGEWIND_DIR=%PROJECT_ROOT%\Edge_Wind_System

if not exist "%EDGEWIND_DIR%\app.py" (
  echo [ERROR] Edge_Wind_System not found: %EDGEWIND_DIR%
  exit /b 1
)

set ENV_FILE=%~dp0.env
if exist "%EDGEWIND_DIR%\edgewind.env" (
  copy /y "%EDGEWIND_DIR%\edgewind.env" "%ENV_FILE%" >nul
) else if not exist "%ENV_FILE%" (
  echo.>"%ENV_FILE%"
)

rem === Icon handling ===
rem The repository's EdgeWind.ico might actually be a PNG (wrong extension).
rem We generate a real ICO wrapper on the fly to ensure Windows + PyInstaller can use it.
set ICON_SRC=%~dp0EdgeWind.ico
set ICON_FILE=%~dp0EdgeWind_build.ico
if exist "%ICON_SRC%" (
  python "%~dp0make_icon.py" "%ICON_SRC%" "%ICON_FILE%"
  if not exist "%ICON_FILE%" (
    set ICON_FILE=%ICON_SRC%
  )
) else (
  set ICON_FILE=
)

set ICON_ARG=
set ISCC_ICON_ARG=
if exist "%ICON_FILE%" (
  set ICON_ARG=--icon "%ICON_FILE%"
  set ISCC_ICON_ARG=/DSetupIcon="%ICON_FILE%"
)

python -m PyInstaller --noconfirm --onefile --windowed ^
  --name "EdgeWind_Monitor" ^
  --distpath "%OUTPUT_ROOT%\dist" ^
  --workpath "%OUTPUT_ROOT%\build" ^
  --paths "%EDGEWIND_DIR%" ^
  --add-data "%EDGEWIND_DIR%\app.py;." ^
  --add-data "%EDGEWIND_DIR%\templates;templates" ^
  --add-data "%EDGEWIND_DIR%\static;static" ^
  --add-data "%EDGEWIND_DIR%\edgewind;edgewind" ^
  --add-data "%ENV_FILE%;." ^
  --collect-submodules "eventlet" ^
  --hidden-import "eventlet.hubs.epolls" ^
  --hidden-import "eventlet.hubs.selects" ^
  --hidden-import "eventlet.hubs.poll" ^
  --hidden-import "eventlet.hubs.kqueue" ^
  --hidden-import "app" ^
  --hidden-import "engineio.async_drivers.threading" ^
  --hidden-import "engineio.async_drivers.eventlet" ^
  --hidden-import "engineio.async_drivers.gevent" ^
  --hidden-import "flask_socketio" ^
  --hidden-import "flask_wtf" ^
  --hidden-import "flask_wtf.csrf" ^
  --hidden-import "webview" ^
  %ICON_ARG% ^
  run_desktop.py

set EXE_PATH=%OUTPUT_ROOT%\dist\EdgeWind_Monitor.exe
set ICON_TARGET=%EXE_PATH%

rem If pyinstaller failed, stop here (avoid building installer with stale exe)
if not exist "%EXE_PATH%" (
  echo [ERROR] PyInstaller failed: %EXE_PATH% not found.
  pause
  exit /b 1
)

if exist "%EXE_PATH%" (
  powershell -NoProfile -Command ^
    "$WshShell = New-Object -ComObject WScript.Shell; " ^
    "$Desktop = [Environment]::GetFolderPath('Desktop'); " ^
    "$Shortcut = $WshShell.CreateShortcut((Join-Path $Desktop 'EdgeWind Monitor.lnk')); " ^
    "$Shortcut.TargetPath = '%EXE_PATH%'; " ^
    "$Shortcut.WorkingDirectory = '%OUTPUT_ROOT%\\dist'; " ^
    "$Shortcut.IconLocation = '%ICON_TARGET%,0'; " ^
    "$Shortcut.Save()"
)

set ISCC_PATH=
where /q iscc && set ISCC_PATH=iscc
if not defined ISCC_PATH if exist "%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe" set ISCC_PATH="%ProgramFiles(x86)%\Inno Setup 6\ISCC.exe"
if not defined ISCC_PATH if exist "%ProgramFiles%\Inno Setup 6\ISCC.exe" set ISCC_PATH="%ProgramFiles%\Inno Setup 6\ISCC.exe"
if not defined ISCC_PATH if exist "D:\Inno Setup 6\ISCC.exe" set ISCC_PATH="D:\Inno Setup 6\ISCC.exe"

if defined ISCC_PATH (
  if exist "%~dp0installer.iss" (
    %ISCC_PATH% %ISCC_ICON_ARG% /O"%OUTPUT_ROOT%\installer" /DDistDir="%OUTPUT_ROOT%\dist" installer.iss
    echo [OK] Installer created in %OUTPUT_ROOT%\installer
  ) else (
    echo [WARN] installer.iss not found. Skipping installer build.
  )
) else (
  echo [WARN] Inno Setup not found. Skipping installer build.
  echo        Install Inno Setup and re-run build_windows.bat
)

echo.
echo [OK] Build finished. Output: %EXE_PATH%
echo.
pause
