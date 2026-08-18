# spdlog 1.15.3 头文件（含捆绑 fmt）。许可证：MIT。https://github.com/gabime/spdlog
# 作为底层实现，业务代码只通过 src/common/w_log.h 的 W_* 宏调用。

INCLUDEPATH += $$PWD/include
DEFINES += SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
