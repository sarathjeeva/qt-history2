######################################################################
# Automatically generated by qmake (2.01a) Tue Aug 29 12:28:05 2006
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += .
CONFIG += qdbus uitools

# Generate interface class
system(dbusxml2cpp -c CarInterface -p car_interface_p.h:car_interface.cpp car.xml)

# Input
FORMS += controller.ui
HEADERS += car_interface_p.h controller.h
SOURCES += main.cpp car_interface.cpp controller.cpp
