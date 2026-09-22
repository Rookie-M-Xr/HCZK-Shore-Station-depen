# ENCViewer SDK 预编译动态库（Qt 6.11.1 / MSVC 2022 / Release）。
# 默认：depen/encviewer/<Qt版本>/<qmake-spec>/
# 可用 ENC_SDK_ROOT 覆盖。只拷 SDK DLL，不拷包内 Qt（与本机套件为同一套 6.11.1）。
# SDK 仅提供 Release 导入库，Debug 不链接，避免混用 Qt6Cored / MD-MDd。

isEmpty(DEPEN_KIT_PRI_LOADED): include($$PWD/../kit.pri)

isEmpty(ENC_SDK_ROOT) {
    ENC_SDK_ROOT = $$clean_path($$PWD/$$QT_VERSION/$$[QMAKE_XSPEC])
}

ENC_SDK_LIB = $$ENC_SDK_ROOT/lib/ENCViewerSdk-Qt6-Release.lib
ENC_SDK_DLL = $$ENC_SDK_ROOT/bin/ENCViewerSdk-Qt6-Release.dll

win32-msvc* {
    exists($$ENC_SDK_LIB):exists($$ENC_SDK_DLL) {
        CONFIG(release, debug|release) {
            DEFINES += SHORE_HAS_ENC_SDK ENCVIEWER_SDK_USE_SHARED=1
            INCLUDEPATH += $$ENC_SDK_ROOT/include $$ENC_SDK_ROOT/include/ENCViewerSdk
            LIBS += $$shell_path($$ENC_SDK_LIB)
            QT += openglwidgets concurrent
            ENC_DLL_DST = $$shell_path($$BIN_OUTPUT_DIR/ENCViewerSdk-Qt6-Release.dll)
            QMAKE_POST_LINK += $$escape_expand(\\n\\t) $$QMAKE_COPY /Y $$shell_quote($$shell_path($$ENC_SDK_DLL)) $$shell_quote($$ENC_DLL_DST)
            message(ENCViewer SDK: $$ENC_SDK_ROOT (Qt 6.11.1 Release DLL + host $$QT_VERSION))
        } else {
            message(ENCViewer SDK skipped in Debug; SDK 包仅有 Release)
        }
    } else {
        warning(ENCViewer SDK not found at $$ENC_SDK_ROOT)
    }
}
