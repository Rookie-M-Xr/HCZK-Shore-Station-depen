# QCustomPlot 2.1.1 预编译静态库（https://www.qcustomplot.com/）
# 构建环境：Qt 6.11.1 + MinGW 13.1（qmake spec: win32-g++）
# 应用工程只引入头文件并链接 .a，不要把 qcustomplot.h 加入 HEADERS，否则会再 moc 一遍导致重复符号。

QT += printsupport
INCLUDEPATH += $$PWD/include
OTHER_FILES += $$PWD/include/qcustomplot.h

win32-g++ {
    QCUSTOMPLOT_LIBDIR = $$PWD/lib/win32-g++
    LIBS += -L$$QCUSTOMPLOT_LIBDIR
    CONFIG(debug, debug|release) {
        LIBS += -lqcustomplotd
        PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/libqcustomplotd.a
    } else {
        LIBS += -lqcustomplot
        PRE_TARGETDEPS += $$QCUSTOMPLOT_LIBDIR/libqcustomplot.a
    }
} else {
    error("depen/qcustomplot 尚未提供当前编译器套件的预编译库。请将对应 .a/.lib 放到 depen/qcustomplot/lib/<qmake-spec>/")
}
