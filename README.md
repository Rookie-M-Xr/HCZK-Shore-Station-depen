# ShoreStation depen

岸基 ShoreStation 的第三方依赖仓库：只保留头文件、qmake `.pri` 和预编译库，不含上游完整源码。

预编译库按 **Qt 版本 + qmake spec** 分目录（见 `kit.pri`）：

- 优先：`<包>/6.11.1/win32-msvc/` 或 `<包>/lib/6.11.1/win32-msvc/`
- 兼容旧 MinGW：`<包>/lib/win32-g++/`

当前包含：

| 依赖 | 类型 | 需要按编译器分库 |
|---|---|---|
| nlohmann/json | 仅头文件 | 否 |
| spdlog | 仅头文件 | 否 |
| QCustomPlot 2.1.1 | 静态库 | 是 |
| QtMqtt 6.11.1 | 动态库 | 是 |
| ENCViewerSdk 1.10（SeaMap，Qt 6.11.1 Release） | 动态库，路径在仓库旁 `../SeaMap/qt-6.11.1-msvc2022_64` | 仅 MSVC Release |
| netCDF-C 4.9.3 | 动态库，默认 `../../WeatherLib/netCDF4.9.3`，可用 `NETCDF_ROOT` 覆盖；**不链 WeatherLib** | 仅 MSVC Release |
| HczkProtocol | 静态库；真源 `../share/libs/HczkProtocol`，MSVC/Qt6 产物在 `../share/dist/`，本目录为拷贝 | 仅 MSVC |

由主体仓库 https://github.com/Rookie-M-Xr/HCZK-Shore-Station 以 git submodule 方式锁定到本仓库的某个 commit。
