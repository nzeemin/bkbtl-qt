# bkbtl-qt
[![License: LGPL v3](https://img.shields.io/badge/License-LGPL%20v3-blue.svg)](https://www.gnu.org/licenses/lgpl-3.0)
[![Build status](https://ci.appveyor.com/api/projects/status/a4y606e4i3xnrnk1?svg=true)](https://ci.appveyor.com/project/nzeemin/bkbtl-qt)
[![Build Status](https://github.com/nzeemin/bkbtl-qt/actions/workflows/push-matrix.yml/badge.svg?branch=master)](https://github.com/nzeemin/bkbtl-qt/actions/workflows/push-matrix.yml)
[![CodeFactor](https://www.codefactor.io/repository/github/nzeemin/bkbtl-qt/badge)](https://www.codefactor.io/repository/github/nzeemin/bkbtl-qt)

BKBTL emulator, Qt version.

The BKBTL project consists of:
* [**bkbtl**](https://github.com/nzeemin/bkbtl) — Win32 version, for Windows.
* [**bkbtl-qt**](https://github.com/nzeemin/bkbtl-qt) is Qt based BKBTL branch, works under Windows, Linux and Mac OS X.
* [**bkbtl-testbench**](https://github.com/nzeemin/bkbtl-testbench) — test bench for regression testing.
* [**bkbtl-doc**](https://github.com/nzeemin/bkbtl-doc) — documentation and screenshots.
* Project wiki: https://github.com/nzeemin/bkbtl-doc/wiki


## Как запустить под Linux

### Собрать из исходников

 1. Установить пакеты: Qt 5 + QtScript<br>
    `sudo apt install qtbase5-dev qt5-qmake qtscript5-dev`
 2. Скачать исходники: либо через<br>
    `git clone https://github.com/nzeemin/bkbtl-qt.git`<br>
    либо скачать как .zip и распаковать
 3. Выполнить команды:<br>
   `cd emulator`<br>
   `qmake "CONFIG+=release" QtBkBtl.pro`<br>
   `make`<br>
 4. Дать права на выполнение: `chmod +x QtBkBtl`
 5. Запустить `QtBkBtl`
 6. Если при запуске появилось сообщение вида `Failed to load Monitor ROM file.`, то
    скачать .rom файлы от [BKBTL](https://github.com/nzeemin/bkbtl/tree/master/roms), положить в ту же папку где лежит файл `QtBkBtl`, запустить снова

### Используя готовый AppImage

 1. Зайти в [Releases](https://github.com/nzeemin/bkbtl-qt/releases) найти последний AppImage-релиз и скачать `*.AppImage` файл
 2. Дать права на выполнение: `chmod +x BKBTL_Qt-9cc9d83-x86_64.AppImage` (подставить тут правильное название AppImage файла)
 3. Запустить AppImage файл
 4. Если при запуске появилось сообщение вида `Failed to load Monitor ROM file.`, то
    скачать .rom файлы от [BKBTL](https://github.com/nzeemin/bkbtl/tree/master/roms), положить в ту же папку где лежит AppImage файл, запустить снова
