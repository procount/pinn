#-------------------------------------------------
#
# Project created by QtCreator 2013-04-30T12:10:31
#
#-------------------------------------------------

QT       += core gui network dbus widgets multimedia

RPI_USERLAND_DIR=../../host/arm-buildroot-linux-uclibcgnueabi/sysroot/usr
exists($${RPI_USERLAND_DIR}/include/interface/vmcs_host/vc_cecservice.h) {
    INCLUDEPATH += $${RPI_USERLAND_DIR}/include $${RPI_USERLAND_DIR}/include/interface/vcos/pthreads
    LIBS += -lbcm_host -lvcos -lvchiq_arm -lvchostif -L$${RPI_USERLAND_DIR}/lib -lrt -ldl
    DEFINES += RASPBERRY_CEC_SUPPORT
} else {
    message(Disabling CEC support for Raspberry Pi - rpi-userland headers not found at $${RPI_USERLAND_DIR})
}


TARGET = recovery
TEMPLATE = app
LIBS += -lqjson-qt5

system(sh updateqm.sh 2>/dev/null)

SOURCES += \
    main.cpp\
    mainwindow.cpp \
    languagedialog.cpp \
    initdrivethread.cpp \
    keydetection.cpp \
    gpioinput.cpp \
    progressslideshowdialog.cpp \
    confeditdialog.cpp \
    rightbuttonfilter.cpp \
    json.cpp \
    multiimagewritethread.cpp \
    util.cpp \
    twoiconsdelegate.cpp \
    bootselectiondialog.cpp \
    wifisettingsdialog.cpp \
    wpa_supplicant/wpsinterface.cpp \
    wpa_supplicant/wpasupplicant.cpp \
    wpa_supplicant/wpafactory.cpp \
    wpa_supplicant/network.cpp \
    wpa_supplicant/interface.cpp \
    wpa_supplicant/bss.cpp \
    osinfo.cpp \
    partitioninfo.cpp \
    longpresshandler.cpp \
    passwd.cpp \
    piclonedialog.cpp \
    piclonethread.cpp \
    copythread.cpp \
    builddata.cpp \
    ceclistener.cpp \
    mydebug.cpp \
    osgroup.cpp \
    multiimagedownloadthread.cpp \
    fscheck.cpp \
    fullfatthread.cpp \
    countdownfilter.cpp \
    repair.cpp \
    rerunsetup.cpp \
    replace.cpp \
    splash.cpp \
    backupthread.cpp \
    datetimedialog.cpp \
    backupdialog.cpp \
    iconcache.cpp \
    renamedialog.cpp \
    termsdialog.cpp \
    optionsdialog.cpp \
    input.cpp \
    joystick.cpp \
    simulate.cpp \
    WidgetKeyboard.cpp

HEADERS  += \
    mainwindow.h \
    languagedialog.h \
    initdrivethread.h \
    config.h \
    keydetection.h \
    gpioinput.h \
    mbr.h \
    progressslideshowdialog.h \
    confeditdialog.h \
    rightbuttonfilter.h \
    json.h \
    multiimagewritethread.h \
    util.h \
    twoiconsdelegate.h \
    bootselectiondialog.h \
    wifisettingsdialog.h \
    wpa_supplicant/wpsinterface.h \
    wpa_supplicant/wpasupplicant.h \
    wpa_supplicant/wpafactory.h \
    wpa_supplicant/network.h \
    wpa_supplicant/interface.h \
    wpa_supplicant/bss.h \
    osinfo.h \
    partitioninfo.h \
    longpresshandler.h \
    passwd.h \
    piclonedialog.h \
    piclonethread.h \
    copythread.h \
    builddata.h \
    ceclistener.h \
    mydebug.h \
    osgroup.h \
    multiimagedownloadthread.h \
    fscheck.h \
    fullfatthread.h \
    countdownfilter.h \
    repair.h \
    rerunsetup.h \
    sleepsimulator.h \
    replace.h \
    splash.h \
    backupthread.h \
    datetimedialog.h \
    backupdialog.h \
    iconcache.h \
    renamedialog.h \
    termsdialog.h \
    optionsdialog.h \
    input.h \
    joystick.h \
    simulate.h \
    WidgetKeyboard.h

FORMS    += \
    mainwindow.ui \
    languagedialog.ui \
    progressslideshowdialog.ui \
    confeditdialog.ui \
    bootselectiondialog.ui \
    wifisettingsdialog.ui \
    passwd.ui \
    piclonedialog.ui \
    fscheck.ui \
    repair.ui \
    rerunsetup.ui \
    replace.ui \
    datetimedialog.ui \
    backupdialog.ui \
    renamedialog.ui \
    termsdialog.ui \
    optionsdialog.ui \
    WidgetKeyboard.ui

RESOURCES += \
    icons.qrc

TRANSLATIONS += \
    translation_ae.ts \
    translation_ast.ts \
    translation_ca.ts \
    translation_de.ts \
    translation_en.ts \
    translation_es.ts \
    translation_eu.ts \
    translation_fi.ts \
    translation_fr.ts \
    translation_hu.ts \
    translation_it.ts \
    translation_ja.ts \
    translation_ko.ts \
    translation_no.ts \
    translation_nl.ts \
    translation_pl.ts \
    translation_pt.ts \
    translation_ru.ts \
    translation_sv.ts \
    translation_tr.ts \
    translation_zh_TW.ts

OTHER_FILES += \
    README.txt \
    wpa_supplicant/wpa_supplicant.xml \
    ../buildroot/package/recovery/init \
    recalbox_os_list.json
