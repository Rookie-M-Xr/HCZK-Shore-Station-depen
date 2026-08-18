# 第三方依赖入口。depen 内只保留头文件与预编译库，不包含第三方源码。
# 本目录作为 ShoreStation 的 git submodule 使用，以锁定各人链接的库版本。
include($$PWD/qcustomplot/qcustomplot.pri)
include($$PWD/nlohmann/nlohmann.pri)
include($$PWD/spdlog/spdlog.pri)
include($$PWD/qtmqtt/qtmqtt.pri)
