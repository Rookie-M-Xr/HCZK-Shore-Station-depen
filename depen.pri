# 第三方依赖入口。depen 内只保留头文件与预编译库，不包含第三方源码。
# 预编译库按 Qt 版本 + qmake spec 分目录，见 kit.pri。
# 本目录作为 ShoreStation 的 git submodule 使用，以锁定各人链接的库版本。
include($$PWD/kit.pri)
include($$PWD/qcustomplot/qcustomplot.pri)
include($$PWD/nlohmann/nlohmann.pri)
include($$PWD/spdlog/spdlog.pri)
include($$PWD/qtmqtt/qtmqtt.pri)
include($$PWD/encviewer/encviewer.pri)
include($$PWD/netcdf/netcdf.pri)
include($$PWD/hczkprotocol/hczkprotocol.pri)
