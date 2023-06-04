@echo off
setlocal enabledelayedexpansion

pushd "%~dp0"

set msbuild="%PROGRAMFILES(x86)%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\msbuild.exe"

for /f "usebackq tokens=*" %%i in (`vswhere -latest -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe`) do (
  set msbuild="%%i"
)

if exist build rd /s /q bin

%msbuild% -m -p:Configuration=Debug   -p:Platform=x86 ..\..\micore.sln -t:micore
%msbuild% -m -p:Configuration=Release -p:Platform=x86 ..\..\micore.sln -t:micore

%msbuild% -m -p:Configuration=Debug   -p:Platform=x64 ..\..\micore.sln -t:micore
%msbuild% -m -p:Configuration=Release -p:Platform=x64 ..\..\micore.sln -t:micore

%msbuild% -m -p:Configuration=Debug   -p:Platform=ARM64 ..\..\micore.sln -t:micore
%msbuild% -m -p:Configuration=Release -p:Platform=ARM64 ..\..\micore.sln -t:micore

:exit
popd
@echo on
