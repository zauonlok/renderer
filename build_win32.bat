@echo off

for /f "delims=" %%i in ('tools\win32\vswhere -latest -property installationPath') do (
    set VS000COMNTOOLS=%%i\Common7\Tools\
)
for /f "delims=" %%i in ('tools\win32\vswhere -latest -property displayName') do (
    set VS000COMNTITLE=%%i
)

if defined VS000COMNTOOLS (
    title %VS000COMNTITLE%
    call "%VS000COMNTOOLS%VsDevCmd.bat" -no_logo -arch=amd64 -host_arch=amd64
) else if defined VS140COMNTOOLS (
    title Visual Studio 2015
    call "%VS140COMNTOOLS%vsvars32.bat"
) else if defined VS120COMNTOOLS (
    title Visual Studio 2013
    call "%VS120COMNTOOLS%vsvars32.bat"
) else if defined VS110COMNTOOLS (
    title Visual Studio 2012
    call "%VS110COMNTOOLS%vsvars32.bat"
) else if defined VS100COMNTOOLS (
    title Visual Studio 2010
    call "%VS100COMNTOOLS%vsvars32.bat"
) else (
    title Visual Studio None
    echo Could not find Visual Studio.
    goto :pause
)

set DEFS=/D_CRT_SECURE_NO_WARNINGS
set OPTS=/nologo /W4 /O2 /GL /fp:fast
set SRCS=main.c platforms/win32.c core/*.c scenes/*.c shaders/*.c tests/*.c
set LIBS=gdi32.lib user32.lib

cd renderer && cl /Fe../Viewer %DEFS% %OPTS% %SRCS% %LIBS% && del *.obj && cd ..

:pause
pause
