@echo off
REM ============================================================
REM  WinCry GUI 一键构建脚本
REM  用法: 在 VS 开发者命令提示符(或 vcvars64 环境)中运行
REM        build_release.cmd       构建 Release|x64
REM        build_release.cmd Debug 构建 Debug|x64(需先有 Debug 库)
REM  内核库位置: 默认取本脚本同级目录下的 wencry 及其 build_vs,
REM  可用环境变量 WENCRY_ROOT / WENCRY_BUILD 覆盖(与 WinCryGUI.props 一致)。
REM ============================================================

setlocal

if "%WENCRY_ROOT%"=="" set "WENCRY_ROOT=%~dp0..\wencry"
if "%WENCRY_BUILD%"=="" set "WENCRY_BUILD=%WENCRY_ROOT%\build_vs"
set "GUI_PROJ=%~dp0WinCry GUI.vcxproj"

set "CFG=Release"
if /I "%~1"=="Debug" set "CFG=Debug"

echo 内核源码目录(WENCRY_ROOT) : %WENCRY_ROOT%
echo 内核构建目录(WENCRY_BUILD): %WENCRY_BUILD%

if not exist "%WENCRY_ROOT%\kernel\cry.h" (
  echo [错误] 找不到 %WENCRY_ROOT%\kernel\cry.h,请设置 WENCRY_ROOT 指向 wencry 源码目录
  exit /b 1
)
if not exist "%WENCRY_BUILD%" (
  echo [错误] 找不到构建目录 %WENCRY_BUILD%,请先执行:
  echo   cmake -S "%WENCRY_ROOT%" -B "%WENCRY_BUILD%" -G "Visual Studio 17 2022" -A x64 -DBUILD_TEST=OFF
  exit /b 1
)

echo === [1/2] 构建内核+工具库 (%CFG%) ===
cmake --build "%WENCRY_BUILD%" --config %CFG% --target Wenkernel CMDvals -- /m:1
if errorlevel 1 goto :err

echo === [2/2] 构建 GUI (%CFG%|x64) ===
msbuild "%GUI_PROJ%" /p:Configuration=%CFG% /p:Platform=x64 /t:Rebuild /m
if errorlevel 1 goto :err

echo.
echo === 完成: %~dp0x64\%CFG%\WinCry GUI.exe ===
goto :eof

:err
echo.
echo *** 构建失败,请检查上方错误 ***
exit /b 1
