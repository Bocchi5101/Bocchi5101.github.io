@echo off
setlocal
cd /d "%~dp0"
set "IOT_NODE=node.exe"
where node.exe >nul 2>nul
if errorlevel 1 set "IOT_NODE=%USERPROFILE%\.cache\codex-runtimes\codex-primary-runtime\dependencies\node\bin\node.exe"
if not exist "dist\index.html" (
  echo Build files missing. Run npm install and npm run build first.
  pause
  exit /b 1
)
echo Opening IoT Dashboard. Keep this window open while using the dashboard.
"%IOT_NODE%" node_modules\vite\bin\vite.js preview --configLoader native --host 127.0.0.1 --port 4174 --open
pause
