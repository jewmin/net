@echo off

set config=Debug
set tag=OFF
set vld=OFF

:next-arg
if "%1"=="" goto args-done
if /i "%1"=="debug"        set config=Debug&goto arg-ok
if /i "%1"=="release"      set config=Release&goto arg-ok
if /i "%1"=="vld"          set vld=ON&goto arg-ok
:arg-ok
shift
goto next-arg
:args-done

if "%config%"=="Release" goto build-release

set tag=OFF

goto build-run

:build-release

set tag=ON

:build-run
mkdir build\%config%
cd build/%config%
cmake ../.. -G "Visual Studio 15 2017 Win64" -DRELEASE=%tag% -DVLD=%vld%
cmake --build . --config %config%
cd ../..

:exit
