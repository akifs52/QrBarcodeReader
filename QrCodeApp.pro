QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_core4100
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_core4100
else:unix: LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_core4100

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_objdetect4100
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_objdetect4100
else:unix: LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_objdetect4100

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_videoio4100
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_videoio4100
else:unix: LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_videoio4100

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_imgproc4100
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_imgproc4100
else:unix: LIBS += -L$$PWD/'../../opencv 4.10 extra install/install/x64/vc17/lib/' -lopencv_imgproc4100


INCLUDEPATH += $$PWD/'../../opencv 4.10 extra install/install/include'
DEPENDPATH += $$PWD/'../../opencv 4.10 extra install/install/include'

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle.properties \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    android/res/xml/qtprovider_paths.xml

contains(ANDROID_TARGET_ARCH,arm64-v8a) {
    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
