@echo off
setlocal

REM vulkan shader build
pushd 
cd Vulkan_Project/shaders
call BuildShader.bat
popd

pushd "%~dp0"
mkdir build > NUL 2>&1
cd build
cmake ../ -G "Visual Studio 15 2017 Win64"
nuget install Vulkan_Project/packages.config -OutputDirectory packages
popd

pause