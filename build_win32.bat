@echo off

if defined VS140COMNTOOLS (
    echo Visual Studio 2015
    call "%VS140COMNTOOLS%vsvars32.bat"
) else if defined VS120COMNTOOLS (
    echo Visual Studio 2013
    call "%VS120COMNTOOLS%vsvars32.bat"
) else if defined VS110COMNTOOLS (
    echo Visual Studio 2012
    call "%VS110COMNTOOLS%vsvars32.bat"
) else if defined VS100COMNTOOLS (
    echo Visual Studio 2010
    call "%VS100COMNTOOLS%vsvars32.bat"
) else if defined VS90COMNTOOLS (
    echo Visual Studio 2008
    call "%VS90COMNTOOLS%vsvars32.bat"
) else if defined VS80COMNTOOLS (
    echo Visual Studio 2005
    call "%VS80COMNTOOLS%vsvars32.bat"
) else (
    echo Visual Studio None
    goto :pause
)

set OPTS=/Ferenderer /nologo
set LINK=
set SRCS=platform_win32.c main.c image.c error.c
set LIBS=user32.lib gdi32.lib

cl %OPTS% %SRCS% %LIBS% /link %LINK%
del *.obj

:pause
pause
