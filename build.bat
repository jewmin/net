@echo off

cd %~dp0

set config=Debug

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done
 
set MSBuild="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\"
call %MSBuild%VsMSBuildCmd.bat

if "%config%"=="Release" goto build-release

mkdir build\debug
cd build\debug
cmake ../.. -G "Visual Studio 15 2017 Win64" -DRELEASE=OFF

goto build-run

:build-release

mkdir build\release
cd build\release
cmake ../.. -G "Visual Studio 15 2017 Win64" -DRELEASE=ON

:build-run
msbuild libnet.sln /t:Build /p:Configuration=%config% /p:Platform=x64 /clp:NoSummary;NoItemAndPropertyList;Verbosity=minimal /nologo
cd ../..

:exit
