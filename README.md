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
| ENCViewerSdk 1.10.159（Qt 6.11.1 Release） | 动态库，`encviewer/6.11.1/win32-msvc/`（头文件 + lib + DLL，不含包内 Qt） | 仅 MSVC Release |
| netCDF-C 4.9.3 | 动态库，`netcdf/4.9.3/win32-msvc/`（头文件 + lib + 运行 DLL） | 仅 MSVC Release |
| HczkProtocol | 静态库；真源 `../share/libs/HczkProtocol`，MSVC/Qt6 产物在 `../share/dist/`，本目录为拷贝 | 仅 MSVC |
| crash | 源码 + backward-cpp 头文件，见下方用法 | 否（Windows 链接 dbghelp、psapi） |

由主体仓库 https://github.com/Rookie-M-Xr/HCZK-Shore-Station 以 git submodule 方式锁定到本仓库的某个 commit。

## crash

进程崩溃时把调用栈写到指定目录。源码在 `crash/`，符号解析使用同目录的 `backward.hpp`（backward-cpp，MIT）。不要在业务翻译单元里 include `backward.hpp`。

工程里加入：

```qmake
include(depen/crash/crash.pri)
```

日志就绪后安装一次。`dump_directory_utf8` 是崩溃文件目录；`log_file_name` 是同目录滚动日志文件名，崩溃当下直接往该文件追加一行，不经过日志库；`product_name` 写在崩溃文件首行。`setNoticeLogger` 只用于安装成功、符号缺失和主动打栈，崩溃信号路径不会调用它。

```cpp
hczk::crash::setNoticeLogger(notice, flush);
hczk::crash::InstallOptions options;
options.dump_directory_utf8 = log_dir;
options.log_file_name = "app.log";
options.product_name = "app";
hczk::crash::install(options);
```

崩溃文件名为 `crash_YYYYMMDD_HHMMSS.txt`，其中 `#0` 是崩溃点。MSVC 读取与 exe 同目录的 `.pdb`，MinGW 读取同目录的 `.sym`。主动把当前线程栈写入崩溃文件用 `hczk::crash::logCurrentStack`。
