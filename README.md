# ShoreStation depen

岸基 ShoreStation 的第三方依赖仓库：只保留头文件、qmake `.pri` 和 MinGW 预编译库，不含上游完整源码。

当前包含：

- nlohmann/json
- spdlog
- QCustomPlot
- QtMqtt（MinGW 构建产物）

由主体仓库 https://github.com/Rookie-M-Xr/HCZK-Shore-Station 以 git submodule 方式锁定到本仓库的某个 commit。
