TEMPLATE = app
CONFIG += qt warn_on debug
QT = core gui widgets xml

SOURCES = src/main.cpp \
          src/treeparser.cpp \
          src/function.cpp \
          src/plot.cpp \
          src/dialogs.cpp \
          src/customcontrols.cpp \
          src/mainwindow.cpp

HEADERS = src/common.h \
          src/treeparser.h \
          src/function.h \
          src/plot.h \
          src/dialogs.h \
          src/customcontrols.h \
          src/mainwindow.h

FORMS = src/ui/mainwindow.ui \
        src/ui/colordialog.ui \
        src/ui/exportdialog.ui \
        src/ui/preferencesdialog.ui \
        src/ui/referencedialog.ui \
        src/ui/aboutdialog.ui \
        src/ui/warnunsaveddialog.ui

RESOURCES = resources.qrc

TRANSLATIONS = tr/qmplot_pl.ts

# Everything build-related goes into build/
UI_DIR = build/
MOC_DIR = build/
OBJECTS_DIR = build/
RCC_DIR = build/

DESTDIR = bin/
