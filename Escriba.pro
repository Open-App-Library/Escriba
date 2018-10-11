HEADERS += \
    $$PWD/src/escriba.h \
    $$PWD/src/escriba-textedit.h \
    $$PWD/src/markdownsyntax.h \
    $$PWD/src/edocument.h
SOURCES += \
    $$PWD/src/escriba.cpp \
    $$PWD/src/escriba-textedit.cpp \
    $$PWD/src/markdownsyntax.cpp \
    $$PWD/src/edocument.cpp


FORMS += $$PWD/escriba.ui

DEPENDPATH += $$PWD/src
INCLUDEPATH += $$PWD/src
