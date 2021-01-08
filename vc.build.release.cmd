@echo off

mkdir bin

if exist "%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" (
  call "%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" x86
) else (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x86
)

mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../bin/x86 -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - NMake Makefiles"
cmake --build .
cpack
copy *.zip ..\bin

if errorlevel 1 goto end

cd ..
if exist "%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" (
  call "%VS160COMNTOOLS%..\..\VC\Auxiliary\Build\vcvarsall.bat" x64
) else (
  call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
)
mkdir build64
cd build64
cmake .. -DCMAKE_INSTALL_PREFIX=../bin/x64 -DCMAKE_BUILD_TYPE=Release -G "CodeBlocks - NMake Makefiles"
cmake --build .
cpack
copy *.zip ..\bin

:end
echo See result in directory "bin"
