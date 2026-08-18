# QtMqtt 6.11.1 预编译动态库（https://code.qt.io/cgit/qt/qtmqtt.git）
# 构建环境：Qt 6.11.1 + MinGW 13.1（qmake spec: win32-g++）
# 源码与 MSVC 安装包留在 D:/HCZK_work/tool/mqtt，不要把 qtmqtt 源码编进岸基工程。
# 业务代码禁止直接 include QtMqtt，只通过 communication 的 IMqttClient。

QT += network websockets
INCLUDEPATH += $$PWD/include
DEFINES += QT_MQTT_LIB

win32-g++ {
    QTMQTT_LIBDIR = $$PWD/lib/win32-g++
    LIBS += -L$$QTMQTT_LIBDIR -lQt6Mqtt
    PRE_TARGETDEPS += $$QTMQTT_LIBDIR/libQt6Mqtt.a

    QTMQTT_DLL_SRC = $$PWD/bin/Qt6Mqtt.dll
    QTMQTT_DLL_DST = $$BIN_OUTPUT_DIR/Qt6Mqtt.dll
    QMAKE_POST_LINK += $$escape_expand(\\n\\t)cmd /c copy /y $$shell_path($$QTMQTT_DLL_SRC) $$shell_path($$QTMQTT_DLL_DST)
} else {
    error("depen/qtmqtt 尚未提供当前编译器套件的预编译库。请用 MinGW 重编后放到 depen/qtmqtt/lib/<qmake-spec>/")
}
