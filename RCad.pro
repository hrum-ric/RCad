TARGET   = RCad
TEMPLATE = app

QT     += core gui widgets
CONFIG += c++11

##############################################################
# Build directory
DESTDIR = build/release

DLLDESTDIR  = $$DESTDIR
OBJECTS_DIR = $$DESTDIR/.obj
MOC_DIR     = $$DESTDIR/.moc
RCC_DIR     = $$DESTDIR/.qrc
UI_DIR      = $$DESTDIR/.ui
RE2C_DIR    = $$DESTDIR/.re
LEMON_DIR 	= $$DESTDIR/.lemon

##############################################################
# Scintilla
CONFIG += qscintilla2
equals( QMAKE_CXX, cl): DEFINES +=  QSCINTILLA_DLL

##############################################################
# Parser
win32:  LEMON_BIN = lemon.exe 
unix:	LEMON_BIN = lemon

lemon.name = lemon
lemon.input = LEMON_SOURCES
lemon.commands = $$system_path($$absolute_path(./tools/$${LEMON_BIN})) -cpp ${QMAKE_FILE_IN}
lemon.output = ${QMAKE_FILE_PATH}/${QMAKE_FILE_BASE}.cpp
lemon.dependency_type = TYPE_C
lemon.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += lemon

##############################################################
SOURCES   		+= $$files(source/*.cpp, recursive=true) 
HEADERS   		+= $$files(source/*.h, recursive=true) 
FORMS     		+= $$files(source/*.ui, recursive=true) 
RESOURCES 		+= source/resource.qrc
LEMON_SOURCES 	+= $$files(source/*.y, recursive=true) 

