@echo off

for /f "delims=" %%i in ('vswhere -latest -property installationPath') do (
    set VS150COMNTOOLS=%%i\Common7\Tools\
)

if defined VS150COMNTOOLS (
    title Visual Studio 2017
    call "%VS150COMNTOOLS%VsDevCmd.bat" /no_logo
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
    goto :pause
)

set OPTS=/Fe../Viewer /Zi /nologo /D_CRT_SECURE_NO_WARNINGS /W4 /O2 /GL
set SRCS=main.c platforms/win32.c core/*.c scenes/*.c shaders/*.c tests/*.c
set LIBS=gdi32.lib user32.lib

cd renderer
cl %OPTS% %SRCS% %LIBS%
del *.obj *.pdb
cd ..

:pause
pause
