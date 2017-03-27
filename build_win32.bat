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
) else (
    echo Visual Studio None
    goto :pause
)

set OPTS=/Feviewer /nologo /W4 /wd4996
set SRCS=main.c platform_win32.c image.c error.c
set LIBS=user32.lib gdi32.lib

cl %OPTS% %SRCS% %LIBS%
del *.obj

:pause
pause
