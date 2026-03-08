QT += core gui widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
CONFIG -= app_bundle

TARGET = OpenCV_CameraMonitor
TEMPLATE = app

# OpenCV 4.8.0 配置
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/usr/local/lib -lopencv_dnn -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_imgproc -lopencv_core 

# 加入所有源文件（包括串口实现文件 uart_master.cpp）
SOURCES += main.cpp            inference.cpp            mainwindow.cpp            uart_master.cpp  # 关键：添加串口实现文件

# 加入所有头文件
HEADERS += mainwindow.h            inference.h            uart_master.h    # 关键：添加串口头文件

DEFINES += QT_DEPRECATED_WARNINGS
