# 崩溃堆栈。源码在本目录，符号解析用 backward-cpp 头文件。
# 其它工程：include(depen/crash/crash.pri)，然后 hczk::crash::install。
# 不要把 backward.hpp 加进 HEADERS，也不要在业务翻译单元 include 它。

INCLUDEPATH += $$PWD/include

HEADERS += \
    $$PWD/include/crash_trace.h

SOURCES += \
    $$PWD/src/crash_trace.cpp

# backward-cpp 在 Windows 上走 DbgHelp / StackWalk64
# MinGW 的 pragma comment 无效，且 DbgHelp 读不了 DWARF，运行时靠 .sym
# MSVC 靠同目录 .pdb，pragma comment 与显式链接都保留
win32-g++ {
    LIBS += -ldbghelp -lpsapi
}
win32-msvc* {
    LIBS += dbghelp.lib psapi.lib
}
