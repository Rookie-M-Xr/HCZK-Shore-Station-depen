# QCustomPlot 2.1.1 预编译静态库（https://www.qcustomplot.com/）
# 目录：lib/<Qt版本>/<qmake-spec>/ ，旧 MinGW 仍可用 lib/win32-g++/
# 应用工程只引入头文件并链接，不要把 qcustomplot.h 加入 HEADERS，否则会再 moc 一遍导致重复符号。

isEmpty(DEPEN_KIT_PRI_LOADED): include($$PWD/../kit.pri)

QT += printsupport
INCLUDEPATH += $$PWD/include
OTHER_FILES += $$PWD/include/qcustomplot.h

QCUSTOMPLOT_LIBDIR = $$depenKitDir($$PWD/lib)
isEmpty(QCUSTOMPLOT_LIBDIR) {
    error("depen/qcustomplot 没有 Qt $$QT_VERSION / $$QMAKE_SPEC 的预编译库。请放到 lib/$$QT_VERSION/$$QMAKE_SPEC/")
}

LIBS += -L$$QCUSTOMPLOT_LIBDIR
CONFIG(debug, debug|release) {
    LIBS += -lqcustomplotd
    win32-msvc: PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/qcustomplotd.lib
    else: PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/libqcustomplotd.a
} else {
    LIBS += -lqcustomplot
    win32-msvc: PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/qcustomplot.lib
    else: PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/libqcustomplot.a
}
