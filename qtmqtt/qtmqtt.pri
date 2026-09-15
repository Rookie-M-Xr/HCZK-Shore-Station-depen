# QtMqtt 6.11.1 预编译动态库（https://code.qt.io/cgit/qt/qtmqtt.git）
# 优先：<Qt版本>/<qmake-spec>/{include,lib,bin}
# 兼容旧 MinGW：include/ + lib/win32-g++/ + bin/
# 源码与完整安装包留在 D:/HCZK_work/tool/mqtt，不要把 qtmqtt 源码编进岸基工程。
# 业务代码禁止直接 include QtMqtt，只通过 communication 的 IMqttClient。

isEmpty(DEPEN_KIT_PRI_LOADED): include($$PWD/../kit.pri)

QT += network websockets
DEFINES += QT_MQTT_LIB

QTMQTT_KIT = $$PWD/$$QT_VERSION/$$[QMAKE_XSPEC]
exists($$QTMQTT_KIT/MANIFEST.txt) {
    INCLUDEPATH += $$QTMQTT_KIT/include
    QTMQTT_LIBDIR = $$QTMQTT_KIT/lib
    QTMQTT_BINDIR = $$QTMQTT_KIT/bin
} else: exists($$PWD/lib/$$[QMAKE_XSPEC]/libQt6Mqtt.a)|exists($$PWD/lib/$$[QMAKE_XSPEC]/Qt6Mqtt.lib) {
    INCLUDEPATH += $$PWD/include
    QTMQTT_LIBDIR = $$PWD/lib/$$[QMAKE_XSPEC]
    exists($$PWD/bin/$$[QMAKE_XSPEC]/Qt6Mqtt.dll) {
        QTMQTT_BINDIR = $$PWD/bin/$$[QMAKE_XSPEC]
    } else {
        QTMQTT_BINDIR = $$PWD/bin
    }
} else {
    error("depen/qtmqtt 没有 Qt $$QT_VERSION / $$[QMAKE_XSPEC] 的预编译库。请放到 $$QT_VERSION/$$[QMAKE_XSPEC]/")
}

LIBS += -L$$QTMQTT_LIBDIR
win32-msvc {
    CONFIG(debug, debug|release) {
        LIBS += -lQt6Mqttd
        PRE_TARGETDEPS += $$QTMQTT_LIBDIR/Qt6Mqttd.lib
        QTMQTT_DLL_SRC = $$QTMQTT_BINDIR/Qt6Mqttd.dll
        QTMQTT_DLL_DST = $$BIN_OUTPUT_DIR/Qt6Mqttd.dll
    } else {
        LIBS += -lQt6Mqtt
        PRE_TARGETDEPS += $$QTMQTT_LIBDIR/Qt6Mqtt.lib
        QTMQTT_DLL_SRC = $$QTMQTT_BINDIR/Qt6Mqtt.dll
        QTMQTT_DLL_DST = $$BIN_OUTPUT_DIR/Qt6Mqtt.dll
    }
} else {
    LIBS += -lQt6Mqtt
    PRE_TARGETDEPS += $$QTMQTT_LIBDIR/libQt6Mqtt.a
    QTMQTT_DLL_SRC = $$QTMQTT_BINDIR/Qt6Mqtt.dll
    QTMQTT_DLL_DST = $$BIN_OUTPUT_DIR/Qt6Mqtt.dll
}

QMAKE_POST_LINK += $$escape_expand(\\n\\t)cmd /c copy /y $$shell_path($$QTMQTT_DLL_SRC) $$shell_path($$QTMQTT_DLL_DST)
