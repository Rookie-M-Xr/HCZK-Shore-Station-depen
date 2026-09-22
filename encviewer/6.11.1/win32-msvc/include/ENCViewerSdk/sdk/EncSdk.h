#pragma once

#include "EncViewerSdkGlobal.h"

#include <QString>

/** @brief ENCViewer SDK 初始化参数。 */
struct EncSdkOptions
{
    QString cacheDirectory; ///< idx、cache 工作目录；为空时使用可执行程序目录下的 WorkFile/MapCache，属性数据库固定为该目录下的 cache.sqlite。
};

/** @brief ENCViewer SDK 初始化结果。 */
struct EncSdkInitializeResult
{
    bool success = false; ///< 是否初始化成功。
    QString errorMessage; ///< 初始化失败原因。
};

/** @brief ENCViewer SDK 进程级初始化与关闭入口。 */
class ENCVIEWER_SDK_EXPORT EncSdk final
{
public:
    /**
     * @brief 初始化 SDK 运行环境。
     * @param options 缓存目录配置；省略或目录为空时使用可执行程序目录下的 WorkFile/MapCache，与当前工作目录及应用名称无关。S-52 和 GDAL 数据固定使用内置资源。
     * @note 默认目录必须可写；受限安装目录或移动端可显式指定可写目录，不自动回退到其他位置。
     * @return 初始化是否成功及失败原因。
     * @note 调用前必须已经创建 QCoreApplication；创建 MapWidget 前必须完成初始化。
     * @note 必须在应用主线程调用；使用地图控件时需要 QApplication。
     * @note 成功仅表示基础环境就绪，不表示后台数据库完整性检查完成；失败不由 SDK 弹窗。
     * @note 进程内只允许一次完整初始化生命周期。已初始化时重复调用返回成功，忽略新参数；
     *       shutdown 后再次初始化返回失败。首次初始化失败可修正参数后重试。
     */
    static EncSdkInitializeResult initialize(const EncSdkOptions &options = {});

    /**
     * @brief 关闭 SDK 并恢复初始化时修改的进程环境。
     * @note 调用前必须销毁所有 SDK 控件并等待导入任务结束。
     * @note 必须在应用主线程调用，不隐式等待业务任务，不删除磁盘缓存和数据库。
     *       未初始化或已关闭时不执行操作；移动端进入后台不应调用此接口。
     */
    static void shutdown();

    /**
     * @brief 返回 SDK 当前是否已初始化；初始化前、失败后和关闭后为 false。
     * @note 可跨线程查询，但只是查询时刻的状态，不保证后续操作期间 SDK 不会关闭，亦不表示后台任务完成。
     */
    static bool isInitialized();

    /** @brief 返回编译时 SDK 版本字符串，初始化前和关闭后均可调用。 */
    static QString version();

private:
    EncSdk() = delete;
};
