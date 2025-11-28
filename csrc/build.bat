@echo off
rem Build quickkart.dll (MinGW should be in PATH)
set SRC_DIR=%~dp0
echo Building DLL in %SRC_DIR%
gcc -O2 -shared -o quickkart.dll products.c users.c orders.c invoice.c recommendation.c tickets.c ticket_replies.c utils.c inventory.c -I .
if errorlevel 1 (
  echo Build failed. See gcc output above.
  pause
  exit /b 1
)
rem copy to web csrc
set TARGET=%SRC_DIR: =%
set WEB_DIR=%SRC_DIR%\..\web\csrc
if not exist "%WEB_DIR%" mkdir "%WEB_DIR%"
copy /Y quickkart.dll "%WEB_DIR%\quickkart.dll"
echo Build succeeded. DLL copied to %WEB_DIR%
pause
