@echo on
mkdir build
cd build
qmake -spec win32-msvc2015 -tp vc ..\RCad.pro
cd ..