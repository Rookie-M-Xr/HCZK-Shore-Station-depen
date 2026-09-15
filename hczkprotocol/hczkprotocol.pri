# HczkProtocol 预编译静态库。
# 真源在仓库旁 share/libs/HczkProtocol；MSVC/Qt6 正式产物在 share/dist/<Qt>/<spec>/。
# 本目录是给岸基链接的拷贝。不要链 BaseCore / CommunicationCore，也不要把 share 源码编进岸基。

isEmpty(DEPEN_KIT_PRI_LOADED): include($$PWD/../kit.pri)

INCLUDEPATH += $$PWD/include

HCZKPROTOCOL_LIBDIR = $$depenKitDir($$PWD/lib)
isEmpty(HCZKPROTOCOL_LIBDIR) {
    error("depen/hczkprotocol 没有 Qt $$QT_VERSION / $$[QMAKE_XSPEC] 的预编译库。请先在 share 用 MSVC/Qt6 编 HczkProtocol，再拷到 lib/$$QT_VERSION/$$[QMAKE_XSPEC]/")
}

LIBS += -L$$HCZKPROTOCOL_LIBDIR
win32-msvc {
    CONFIG(debug, debug|release) {
        LIBS += -lHczkProtocol_d
        PRE_TARGETDEPS += $$HCZKPROTOCOL_LIBDIR/HczkProtocol_d.lib
    } else {
        LIBS += -lHczkProtocol
        PRE_TARGETDEPS += $$HCZKPROTOCOL_LIBDIR/HczkProtocol.lib
    }
} else {
    error("depen/hczkprotocol 当前只提供 win32-msvc 预编译库")
}
