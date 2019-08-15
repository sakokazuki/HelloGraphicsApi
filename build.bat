@echo off
setlocal

pushd "%~dp0"
mkdir build > NUL 2>&1
cd build
cmake ../ -G "Visual Studio 15 2017 Win64"
nuget restore ./HelloGraphicsApi.sln
popd

pause