@echo off
set ASTYLEEXE=c:\bin\astyle.exe
set ASTYLEOPT=-n -Q --options=..\astyle-cpp-options
%ASTYLEEXE% %ASTYLEOPT% *.h *.cpp --exclude=ui_mainwindow.h
%ASTYLEEXE% %ASTYLEOPT% Util\*.h Util\*.cpp
