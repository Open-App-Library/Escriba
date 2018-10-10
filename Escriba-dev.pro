#---------------------------------------------------------#
# Escriba-dev.pro is useful if you are developing Escriba #
# instead of integrating Escriba into your existing Qt    #
# project.                                                #
#---------------------------------------------------------#

include(Escriba.pro)
SOURCES += example.cpp

QT       += core gui widgets

TARGET = Escriba
TEMPLATE = app
DEFINES += QT_DEPRECATED_WARNINGS
CONFIG += c++11

RESOURCES += \
    resources/escriba-icons.qrc
