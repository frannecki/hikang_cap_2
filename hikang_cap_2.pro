TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
SOURCES += main.cpp

INCLUDEPATH+=C:\opencv2\include\opencv\
                    C:\opencv2\include\opencv2\
                    C:\opencv2\include\
                    D:\Documents\Hikvision\CH-HCNetSDK\
                    C:\Anaconda2\include


LIBS+=C:\opencv2\x86\mingw\lib\libopencv_calib3d2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_core2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_flann2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_highgui2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_imgproc2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_ml2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_features2d2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_objdetect2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_video2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_ocl2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_nonfree2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_photo2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_stitching2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_superres2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_ts2413.a\
        C:\opencv2\x86\mingw\lib\libopencv_videostab2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_gpu2413.dll.a\
        C:\opencv2\x86\mingw\lib\libopencv_contrib2413.dll.a\
        D:\Documents\Hikvision\CH-HCNetSDK\HCCore.lib\
        D:\Documents\Hikvision\CH-HCNetSDK\HCNetSDK.lib\
        D:\Documents\Hikvision\CH-HCNetSDK\HCAlarm.lib\
        D:\Documents\Hikvision\CH-HCNetSDK\HCGeneralCfgMgr.lib\
        D:\Documents\Hikvision\CH-HCNetSDK\HCPreview.lib\
        D:\Documents\Hikvision\CH-HCNetSDK\PlayCtrl.lib\
        C:\Anaconda2\libs\python27.lib

HEADERS += \
    pycap.h \
    pycap.h
